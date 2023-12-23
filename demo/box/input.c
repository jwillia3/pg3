#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-box.h>

struct key_map {
    const char* key;
    void        (*exec)(pgb_t *box);
};

static const pgb_type_t type;
static const struct key_map key_map[];


static size_t
_len(pgb_t *box)
{
    return strlen(pgb_prop_get(box, "text"));
}


static size_t
_caret(pgb_t *box)
{
    return fmin(pgb_prop_get_float(box, "caret"), _len(box));
}


static void
_set_caret(pgb_t *box, size_t caret)
{
    pgb_prop_set_float(box, "caret", caret);
}


static void
_splice(pgb_t *box, size_t caret, size_t del_len, const char *insert, size_t insert_len)
{
    const char  *text = pgb_prop_get(box, "text");
    size_t      len = _len(box);
    del_len = caret + del_len > len? len - caret: del_len;
    size_t      new_len = len - del_len + insert_len + 1;
    char        *buf = malloc(new_len);
    snprintf(buf, new_len, "%.*s%.*s%.*s",
        (int) caret,
        text,
        (int) insert_len,
        insert,
        (int) (len - del_len),
        text + caret + del_len);
    pgb_prop_set(box, "text", buf);
    free(buf);
}


static void
_left(pgb_t *box)
{
    unsigned caret = _caret(box);
    if (caret > 0)
        _set_caret(box, caret - 1);
}


static void
_right(pgb_t *box)
{
    unsigned caret = _caret(box);
    if (caret < _len(box))
        _set_caret(box, caret + 1);
}


static void
_home(pgb_t *box)
{
    _set_caret(box, 0);
}


static void
_end(pgb_t *box)
{
    _set_caret(box, _len(box));
}


static void
_backspace(pgb_t *box)
{
    unsigned caret = _caret(box);
    if (caret > 0) {
        _splice(box, caret - 1, 1, NULL, 0);
        _set_caret(box, caret - 1);
    }
}


static void
_delete(pgb_t *box)
{
    _splice(box, _caret(box), 1, NULL, 0);
}


static void
_insert(pgb_t *box, const char *insert)
{
    size_t      caret = _caret(box);
    size_t      insert_len = strlen(insert);
    _splice(box, caret, 0, insert, insert_len);
    _set_caret(box, caret + insert_len);

}


static bool
on_key_down(pgb_t *box, void *etc)
{
    const char  *key = ((pgb_key_down_t*) etc)->key;
    for (const struct key_map *i = key_map; i->key; i++)
        if (!strcmp(i->key, key)) {
            i->exec(box);
            break;
        }
    return false;
}


static bool
on_text(pgb_t *box, void *etc)
{
    _insert(box, ((pgb_text_t*) etc)->text);
    return false;
}

pgb_t*
pgb_input(const char *arg, ...)
{
    pgb_t *box = pgb_box(&type,
        "text", "",
        "caret", "0",
        "ipad", "8",
        "min-x", "200",
        "foreground", "ui-fg",
        "background", "ui-bg",
        "border-color", "ui-fg",
        "border-radius", "8",
        "border-width", "2",
        "can-focus", "true",
        "on-key-down", on_key_down,
        "on-text", on_text,
        NULL);
    va_list ap;
    va_start(ap, arg);
    pgb_props_set_var(box, arg, ap);
    va_end(ap);

    _set_caret(box, strlen(pgb_prop_get(box, "text")));
    return box;
}


static PgPt
measure(const pgb_t *box)
{
    PgFont      *f = pgb_font();
    const char  *text = pgb_prop_get(box, "text");
    float       th = pg_font_get_height(f);
    float       tw = pg_font_measure_string(f, text);
    float       ipad = pgb_prop_get_float(box, "ipad");
    return pgpt(tw + ipad * 2, th + ipad * 2);
}


static void
draw(pgb_t *box, Pg *g)
{
    PgFont      *f = pgb_font();
    float       th = pg_font_get_height(f);
    const char  *text = pgb_prop_get(box, "text");
    const char  *fg = pgb_prop_get(box, "foreground");
    float       ipad = pgb_prop_get_float(box, "ipad");
    float       bw = pgb_prop_get_float(box, "border-width");
    float       br = pgb_prop_get_float(box, "border-radius");
    PgPt        sz = box->size;
    int         caret = pgb_prop_get_float(box, "caret");

    pg_canvas_set_fill(g, pg_paint_from_name(pgb_prop_get(box, "background")));
    if (bw) {
        pg_canvas_set_stroke(g, pg_paint_from_name("border-color"));
        pg_canvas_set_line_width(g, bw);
        pg_canvas_rounded_rectangle(g, bw * .5f, bw * .5f, sz.x - bw, sz.y - bw, br);
        pg_canvas_fill_stroke(g);
    } else {
        pg_canvas_rounded_rectangle(g, 0, 0, sz.x, sz.y, br);
        pg_canvas_fill(g);
    }

    pg_canvas_set_fill(g, pg_paint_from_name(fg));
    pg_canvas_show_string(g, f, ipad, ipad, text);

    if (pgb_is_focus(box)) {
        float       cx = pg_font_measure_chars(f, text, caret);
        pg_canvas_set_stroke(g, pg_paint_from_name("ui-accent"));
        pg_canvas_set_line_width(g, 2);
        pg_canvas_move(g, ipad + cx, ipad);
        pg_canvas_rline(g, 0, th);
        pg_canvas_stroke(g);
    }
}


static const struct key_map
key_map[] = {
    {"Left", _left},
    {"Right", _right},
    {"Home", _home},
    {"End", _end},
    {"Backspace", _backspace},
    {"Delete", _delete},
    {NULL, NULL},
};


static const pgb_type_t
type = {
    "input",
    .measure = measure,
    .draw = draw,
};
