#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>


static const float PX = 12.0f;
static const float PY = 8.0f;
static const float DEF_CHARS = 10.0f;
static const float CARET_WIDTH = 1.5f;

typedef struct Data Data;
typedef struct Undo Undo;

struct Data {
    char    *buf;
    int     len;
    int     cap;
    int     caret;
    int     sel;
    float   camera;
    bool    disabled;
    Undo    *undo;
    Undo    *redo;
    void    (*enter)(Pgb *box, const char *text);
    void    (*updated)(Pgb *box, const char *text);
};

struct Undo {
    int     caret;
    int     len;
    char    *buf;
    Undo    *next;
};

static void delete_undos(Undo **undop);


Pgb*
pgb_on_update_textbox(Pgb *box,
                      void (*updated)(Pgb *box, const char *text))
{
    if (!box) return 0;
    Data *dat = box->sys;
    dat->updated = updated;
    return box;
}


char *
pgb_get_textbox_text(Pgb *box)
{
    if (!box) return 0;
    Data *dat = box->sys;
    return strdup(dat->buf);
}


Pgb*
pgb_set_textbox_text(Pgb *box, const char *text)
{
    if (!box || !text) return box;

    Data *dat = box->sys;

    delete_undos(&dat->undo);
    delete_undos(&dat->redo);
    free(dat->buf);
    dat->buf = strdup(text);
    dat->len = strlen(text);
    dat->cap = dat->len;
    dat->caret = dat->len;
    dat->sel = -1;
    return box;
}


bool
pgb_is_textbox_enabled(Pgb *box)
{
    if (!box) return false;
    Data *dat = box->sys;
    return dat->disabled;
}


Pgb*
pgb_enable_textbox(Pgb *box, bool enabled)
{
    if (!box) return box;
    Data *dat = box->sys;
    dat->disabled = !enabled;
    return box;
}


static
void
delete_undos(Undo **undop)
{
    for (Undo *i = *undop, *next; i; i = next) {
        next = i->next;
        free(i->buf);
        free(i);
    }
    *undop = 0;
}


static
Undo*
new_undo(Data *dat)
{
    return new(Undo,
               .caret = dat->caret,
               .buf = strdup(dat->buf),
               .len = dat->len,
               .next = 0);
}


static
void
push_undo(Undo **ptr, Undo *undo)
{
    if (!undo) return;

    undo->next = *ptr;
    *ptr = undo;
}


static
void
pop_undo(Data *dat, Undo **undop)
{
    Undo *undo = *undop;
    if (!undo)
        return;
    *undop = undo->next;
    free(dat->buf);
    dat->buf = undo->buf;
    dat->caret = undo->caret;
    dat->len = undo->len;
    dat->cap = undo->len;
    free(undo);
}


static
void
record_undo(Data *dat)
{
    delete_undos(&dat->redo);
    push_undo(&dat->undo, new_undo(dat));
}


static
void
undo(Data *dat)
{
    if (!dat->undo)
        return;
    push_undo(&dat->redo, new_undo(dat));
    pop_undo(dat, &dat->undo);
}


static
void
redo(Data *dat)
{
    if (!dat->redo)
        return;
    push_undo(&dat->undo, new_undo(dat));
    pop_undo(dat, &dat->redo);
}


static
void
notify(Pgb *box)
{
    Data *dat = box->sys;

    if (dat->disabled)
        return;

    if (dat->updated)
        dat->updated(box, dat->buf);
}


static
void
enter(Pgb *box)
{
    Data *dat = box->sys;
    if (dat->enter)
        dat->enter(box, dat->buf);
}


static
void
delete_selection(Data *dat)
{
    if (dat->sel == -1)
        return;
    if (dat->disabled)
        return;

    int i = dat->sel < dat->caret? dat->sel: dat->caret;
    int j = dat->sel > dat->caret? dat->sel: dat->caret;
    int n = j - i;

    record_undo(dat);

    memmove(dat->buf + i,
            dat->buf + i + n,
            dat->len - i - n + 1);
    dat->len -= n;
    dat->sel = -1;
    dat->caret = i;
}


static
void
insert(Data *dat, const char *text, int len)
{
    if (!text)
        return;
    if (dat->disabled)
        return;

    record_undo(dat);

    delete_selection(dat);

    if (dat->len + len > dat->cap) {
        dat->cap = dat->len + len + DEF_CHARS;
        dat->buf = realloc(dat->buf, dat->cap + 1);
    }
    memmove(dat->buf + dat->caret + len,
           dat->buf + dat->caret,
           dat->len - dat->caret + 1);
    memcpy(dat->buf + dat->caret, text, len);
    dat->caret += len;
    dat->len += len;
}


static
void
delete(Data *dat, int n)
{
    if (dat->disabled)
        return;

    if (dat->sel != -1)
        delete_selection(dat);
    else {
        record_undo(dat);
        if (dat->caret + n > dat->len)
            n = dat->len - dat->caret;
        memmove(dat->buf + dat->caret,
                dat->buf + dat->caret + n,
                dat->len - dat->caret - n + 1);
        dat->len -= n;
    }
}


static
void
backspace(Data *dat)
{
    if (dat->disabled)
        return;

    if (dat->sel != -1)
        delete_selection(dat);
    else if (dat->caret && dat->len) {
        record_undo(dat);
        dat->caret--;
        memmove(dat->buf + dat->caret,
                dat->buf + dat->caret + 1,
                dat->len - dat->caret);
        dat->len--;
    }
}


static
void
left(Data *dat)
{
    if (dat->caret)
        dat->caret--;
}


static
void
right(Data *dat)
{
    if (dat->caret + 1 <= dat->len)
        dat->caret++;
}


static
void
home(Data *dat)
{
    dat->caret = 0;
}


static
void
end(Data *dat)
{
    dat->caret = dat->len;
}


static
void
sel(Data *dat)
{
    if (dat->sel == -1)
        dat->sel = dat->caret;
}


static
void
unsel(Data *dat)
{
    if (dat->sel != -1)
        dat->sel = -1;
}


static
void
paste(Data *dat)
{
    if (dat->disabled)
        return;

    char *text = pg_get_clipboard();
    if (!text)
        return;

    // Paste one line at a time. Replace CR/LF with space.
    for (char *i = text; *i; ) {
        int n = strcspn(i, "\r\n");
        insert(dat, i, n);
        if (i[n])
            insert(dat, " ", 1);
        i += n;
        while (*i == '\n' || *i == '\r') i++;
    }

    free(text);
}


static
void
copy(Data *dat)
{
    if (dat->sel == -1)
        return;

    int i = dat->sel < dat->caret? dat->sel: dat->caret;
    int j = dat->sel > dat->caret? dat->sel: dat->caret;
    int n = j - i;

    if (n == 0)
        return;

    char *tmp = malloc(n + 1);
    memcpy(tmp, dat->buf + i, n);
    tmp[n] = 0;
    pg_set_clipboard(tmp);
    free(tmp);
}


static
void
autosize(Pgb *box)
{
    PgFont *font = pgb_font(PGB_DEFAULT_FONT);
    float em = pg_measure_char(font, 'M');
    box->cx = ceilf(em * DEF_CHARS + PX * 2.0f);
    box->cy = ceilf(pg_font_height(font) + PY * 2.0f);
}


static
void
hover(Pgb *box, bool over)
{
    (void) over;
    pgb_dirty(box);
}


static
void
mouse_down(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) x, (void) y, (void) button, (void) mods;
    pgb_set_focused(box);
    pgb_dirty(box);
}


static
void
key_down(Pgb *box, PgKey key, PgMods mods)
{
    Data *dat = box->sys;

    if (key == PG_KEY_BACKSPACE && !mods)
        backspace(dat),
        notify(box);
    else if (key == PG_KEY_DELETE && !mods)
        delete(dat, 1),
        notify(box);
    else if (key == PG_KEY_LEFT && !mods)
        unsel(dat),
        left(dat);
    else if (key == PG_KEY_RIGHT && !mods)
        unsel(dat),
        right(dat);
    else if (key == PG_KEY_LEFT && mods == PG_MOD_SHIFT)
        sel(dat),
        left(dat);
    else if (key == PG_KEY_RIGHT && mods == PG_MOD_SHIFT)
        sel(dat),
        right(dat);
    else if ((key == PG_KEY_HOME && !mods) ||
             (key == 'A' && mods == PG_MOD_CTRL))
        home(dat);
    else if ((key == PG_KEY_END && !mods) ||
             (key == 'E' && mods == PG_MOD_CTRL))
        end(dat);
    else if ((key == PG_KEY_HOME && mods == PG_MOD_SHIFT) ||
             (key == 'A' && mods == PG_MOD_CTRL + PG_MOD_SHIFT))
        sel(dat),
        home(dat);
    else if ((key == PG_KEY_END && mods == PG_MOD_SHIFT) ||
             (key == 'E' && mods == PG_MOD_CTRL + PG_MOD_SHIFT))
        sel(dat),
        end(dat);
    else if ((key == 'V' && (mods == PG_MOD_CTRL || mods == PG_MOD_ALT)) ||
             (key == PG_KEY_INSERT && mods == PG_MOD_SHIFT))
        paste(dat);
    else if ((key == 'C' && (mods == PG_MOD_CTRL || mods == PG_MOD_ALT)) ||
             (key == PG_KEY_INSERT && mods == PG_MOD_CTRL))
        copy(dat);
    else if ((key == 'X' && (mods == PG_MOD_CTRL || mods == PG_MOD_ALT)) ||
             (key == PG_KEY_DELETE && mods == PG_MOD_SHIFT))
        copy(dat),
        delete_selection(dat),
        notify(box);
    else if (key == 'Z' && (mods == PG_MOD_CTRL || mods == PG_MOD_ALT))
        undo(dat);
    else if (key == 'Y' && (mods == PG_MOD_CTRL || mods == PG_MOD_ALT))
        redo(dat);
    else if (key == PG_KEY_ENTER)
        enter(box);
    else
        return;

    pgb_dirty(box);
}


static
void
character(Pgb *box, unsigned codepoint)
{
    Data *dat = box->sys;

    if (dat->disabled)
        return;

    char utf8[8];
    *pg_write_utf8(utf8, utf8 + sizeof utf8, codepoint) = 0;
    insert(dat, utf8, strlen(utf8));
    notify(box);
    pgb_dirty(box);
}


static
void
draw(Pgb *box, Pg *g)
{
    Data *dat = box->sys;
    PgFont *font = pgb_font(PGB_DEFAULT_FONT);
    PgPaint *face_colour = dat->disabled? pgb_color(PGB_LIGHT_COLOR):
                           pgb_color(PGB_LIGHT_COLOR);
    PgPaint *text_colour = dat->disabled? pgb_color(PGB_MID_COLOR):
                           pgb_color(PGB_DARK_COLOR);
    PgPaint *line_colour = dat->disabled? pgb_color(PGB_MID_COLOR):
                           pgb_is_focused(box)? pgb_color(PGB_ACCENT_COLOR):
                           pgb_is_hovered(box)? pgb_color(PGB_ACCENT_COLOR):
                           pgb_color(PGB_MID_COLOR);
    PgPaint *caret_colour = dat->disabled? pgb_color(PGB_MID_COLOR):
                            pgb_color(PGB_ACCENT_COLOR);

    float selx = pg_measure_chars(font, dat->buf, dat->sel);
    float carx = pg_measure_chars(font, dat->buf, dat->caret);
    float width = box->sx - PX * 2.0f;
    float height = box->sy - PY * 2.0f;

    // Border.
    pg_rounded(g, 2.5f, 2.5f, box->sx - 4.0f, box->sy - 4.0f, 4.0f, 4.0f);
    pg_set_fill(g, face_colour);
    pg_set_stroke(g, line_colour);
    pg_fill_stroke(g);


    // Snap camera into to caret.
    if (carx >= dat->camera + width)
        dat->camera = fmaxf(0.0f, carx - width + CARET_WIDTH);
    else if (carx < dat->camera)
        dat->camera = carx;

    // Clip to content area.
    pg_set_clip(g, PX, PY, width, height);
    pg_translate(g, PX - dat->camera, PY);

    // Text.
    pg_string_path(g, font, 0.0f, 0.0f, dat->buf);
    pg_set_fill(g, text_colour);
    pg_fill(g);
    pg_restore(g);

    // Caret and selection.
    if (pgb_is_focused(box)) {
        pg_set_line_width(g, CARET_WIDTH);
        pg_move(g, carx, 0.0f);
        pg_rline(g, 0.0f, height);
        pg_set_stroke(g, caret_colour);
        pg_stroke(g);

        if (dat->sel != -1) {
            float ax = fminf(selx, carx);
            float bx = fmaxf(selx, carx);

            PgColor c = caret_colour->colors[0];
            PgPaint p = pg_solid(caret_colour->cspace, c.x, c.y, c.z, 0.25f);
            pg_rectangle(g, ax, 0.0f, bx - ax, height);
            pg_set_fill(g, &p);
            pg_fill(g);
        }
    }
}


static
void
_free(Pgb *box)
{
    Data *dat = box->sys;
    free(dat->buf);
    delete_undos(&dat->undo);
    delete_undos(&dat->redo);
    free(dat);
}

static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = {
        .autosize = autosize,
        .draw = draw,
        .free = _free,
        .hover = hover,
        .mouse_down = mouse_down,
        .key_down = key_down,
        .character = character,
    };

    if (done) return &methods;

    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_textbox(const char *text,
            void (*enter)(Pgb *box, const char *text),
            PgbFlags flags)
{
    Pgb *box = pgb_box(0.0f, 0.0f, flags, methods());

    if (!text)
        text = "";

    int len = (int) strlen(text);

    Data *dat = new(Data,
                    .len = len,
                    .cap = len,
                    .buf = malloc(len + 1),
                    .caret = len,
                    .sel = -1,
                    .enter = enter);

    memcpy(dat->buf, text, len);
    dat->buf[len] = 0;

    box->sys = dat;
    return box;
}
