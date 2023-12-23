#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3/pg.h>

struct conf {
    PgFont*     text;
    PgPaint*    fg;
    PgPaint*    bg;
    PgPaint*    med;
    PgPaint*    accent;
    PgPaint*    accent_fg;
    PgPaint*    clear;
    float       pad;
};

struct state {
    PgWindowEvent*  e;
    Pg*             g;
    Pg*             sub;
    float           caret_x;
    float           caret_y;

    bool            hovering;
    float           x;
    float           y;
    float           sx;
    float           sy;
    float           mouse_x;
    float           mouse_y;
    bool            mouse_down;
    bool            mouse_just_down;
    bool            mouse_just_up;
};

struct io {
    const void*     hot;
    float           mouse_absolute_x;
    float           mouse_absolute_y;
    bool            mouse_down;
    bool            mouse_just_down;
    bool            mouse_just_up;
};

static bool         initialised;
static struct conf  conf;
static struct state st;
static struct io    io;


bool
pgg_init(float dpi)
{
    if (initialised)
        return true;

    initialised = true;
    const char *family_name = "system-ui, ui-sans-serif, sans-serif, any";
    PgFont  *text = pg_font_find(family_name, 300, false);
    pg_font_scale(text, 10 * dpi / 72.0f, 0.0f);

    conf = (struct conf) {
        .text = text,
        .fg = pg_paint_new_solid(PG_LCHAB, .3, .0, .0, 1.),
        .bg = pg_paint_new_solid(PG_LCHAB, .9, .0, .0, 1.),
        .med = pg_paint_new_solid(PG_LCHAB, .6, .0, .0, 1.),
        .accent = pg_paint_new_solid(PG_LCHAB, .3, .3, .7, 1.),
        .accent_fg = pg_paint_new_solid(PG_LCHAB, .9, .3, .7, 1.),
        .clear = pg_paint_new_solid(PG_LCHAB, 0, 0, 0, 0),
        .pad = 8,
    };

    io = (struct io) {
        .mouse_absolute_x = NAN,
        .mouse_absolute_y = NAN,
        .mouse_down = false,
        .mouse_just_down = false,
        .mouse_just_up = false,
    };

    return true;
}


void
pgg_begin(PgWindowEvent *e)
{
    if (!e) return;

    if (!pgg_init(pg_window_get_dpi(e->win).y)) return;

    Pg *g = pg_window_get_canvas(e->win);

    io.mouse_just_down = false;
    io.mouse_just_up = false;

    switch (e->type) {
    case PG_EVENT_MOUSE_DOWN:
        io.mouse_absolute_x = e->mouse.x;
        io.mouse_absolute_y = e->mouse.y;
        if (!strcmp(e->mouse.button, "LeftButton"))
            io.mouse_just_down = true,
            io.mouse_down = true;
        break;
    case PG_EVENT_MOUSE_UP:
        io.mouse_absolute_x = e->mouse.x;
        io.mouse_absolute_y = e->mouse.y;
        if (!strcmp(e->mouse.button, "LeftButton"))
            io.mouse_just_up = true,
            io.mouse_down = false;
        break;
    case PG_EVENT_MOUSE_MOVED:
        io.mouse_absolute_x = e->mouse.x;
        io.mouse_absolute_y = e->mouse.y;
        break;
    default:
        break;
    }

    st = (struct state) {
        .e = e,
        .g = g,
        .mouse_down = io.mouse_down,
        .mouse_just_down = io.mouse_just_down,
        .mouse_just_up = io.mouse_just_up,
    };

    pg_canvas_state_reset(g);
    pg_canvas_set_clear(g, conf.bg);
    pg_canvas_clear(g);
}


void
pgg_end(void)
{
    pg_canvas_commit(st.sub);
    pg_canvas_free(st.sub);
    pg_canvas_commit(st.g);
    pg_window_update(st.e->win);
}


Pg*
pgg_get(float sx, float sy)
{
    float x = st.caret_x;
    float y = st.caret_y;

    sx = ceilf(sx);
    sy = ceilf(sy);

    st.caret_y += sy + conf.pad;

    st.hovering = x <= io.mouse_absolute_x && io.mouse_absolute_x < x + sx &&
             y <= io.mouse_absolute_y && io.mouse_absolute_y < y + sy;
    st.x = x;
    st.y = y;
    st.sx = sx;
    st.sy = sy;
    st.mouse_x = io.mouse_absolute_x - st.x;
    st.mouse_y = io.mouse_absolute_y - st.y;


    pg_canvas_commit(st.sub);
    st.sub = pg_canvas_new_subcanvas(st.g, x, y, sx, sy);
    return st.sub;
}

bool
pgg_set_hot(const void *id)
{
    if (io.hot == id) return true;
    if (!io.hot) return io.hot = id;
    return false;
}


void
pgg_text(const char *text)
{
    PgFont  *f = conf.text;
    float   th = pg_font_get_height(f);
    float   tw = pg_font_measure_string(f, text);
    Pg      *g = pgg_get(tw, th);
    pg_canvas_set_fill(g, conf.fg);
    pg_canvas_show_string(g, f, 0, 0, text);
}


bool
pgg_button(const char *text)
{
    PgFont  *f = conf.text;
    float   th = pg_font_get_height(f);
    float   tw = pg_font_measure_string(f, text);
    float   p = conf.pad;
    Pg      *g = pgg_get(tw + p * 4, th + p * 4);
    PgPt    sz = pg_canvas_get_size(g);

    if (st.hovering && io.mouse_just_down)
        io.hot = text;
    bool    pressed = st.hovering && io.mouse_down && io.hot == text;
    bool    clicked = st.hovering && io.hot == text && io.mouse_just_up;

    PgPaint     *face = pressed? conf.med: conf.clear;
    PgPaint     *border = st.hovering && !pressed? conf.accent: conf.fg;
    PgPaint     *fg = conf.fg;

    pg_canvas_set_fill(g, face);
    pg_canvas_set_stroke(g, border);
    pg_canvas_set_line_width(g, 2);
    pg_canvas_rounded_rectangle(g, 1, 1, sz.x-2, sz.y-2, p);
    pg_canvas_fill_stroke(g);
    pg_canvas_rounded_rectangle(g, 4, 4, sz.x-2*4, sz.y-2*4, p);
    pg_canvas_stroke(g);

    pg_canvas_set_fill(g, fg);
    pg_canvas_show_string(g, f, .5f * (sz.x - tw), .5f * (sz.y - th), text);
    return clicked;
}


bool
pgg_checkbox_or_radio(const char *text, bool checked, bool radio)
{
    PgFont  *f = conf.text;
    float   th = pg_font_get_height(f);
    float   tw = pg_font_measure_string(f, text);
    float   xh = th * .75f;
    float   r = radio? xh: 0.0f;
    Pg      *g = pgg_get(tw + th, th);

    if (st.hovering && io.mouse_just_down)
        io.hot = text;
    bool    pressed = st.hovering && io.mouse_down && io.hot == text;
    bool    clicked = st.hovering && io.hot == text && io.mouse_just_up;

    if (clicked)
        checked = !checked;

    PgPaint     *face = checked || pressed? conf.accent: conf.clear;
    PgPaint     *border = conf.fg;
    PgPaint     *fg = st.hovering? conf.accent: conf.fg;

    pg_canvas_set_fill(g, face);
    pg_canvas_set_stroke(g, border);
    pg_canvas_set_line_width(g, 2);
    pg_canvas_rounded_rectangle(g, .5f*(th - xh), .5f*(th - xh), xh - 2, xh - 2, r);
    pg_canvas_fill_stroke(g);

    pg_canvas_set_fill(g, fg);
    pg_canvas_show_string(g, f, th, 0.0f, text);
    return radio? clicked: checked;
}


bool
pgg_checkbox(const char *text, bool checked)
{
    return pgg_checkbox_or_radio(text, checked, false);
}


bool
pgg_radio(const char *text, bool checked)
{
    return pgg_checkbox_or_radio(text, checked, true);
}


const char*
pgg_listbox(const char **const items, unsigned count, const char *selected)
{
    PgFont  *f = conf.text;
    float   fh = pg_font_get_height(f);
    float   tw = 0;
    for (unsigned i = 0; i < count; i++)
        tw = fmaxf(tw, pg_font_measure_string(f, items[i]));

    float   p = conf.pad;
    Pg      *g = pgg_get(tw + 2*p, fh * count);
    PgPt    sz = pg_canvas_get_size(g);

    float y = 0.0;
    pg_canvas_set_fill(g, conf.fg);
    for (unsigned i = 0; i < count; i++) {
        if (y > sz.y) break;

        bool cur_selected = !strcmp(selected, items[i]);
        bool hovering = st.hovering && y <= st.mouse_y && st.mouse_y < y + fh;

        if (hovering && st.mouse_just_down)
            selected = items[i],
            cur_selected = true;

        if (cur_selected || hovering) {
            pg_canvas_set_fill(g, cur_selected? conf.accent: conf.clear);
            pg_canvas_set_stroke(g, cur_selected || hovering? conf.accent: conf.clear);
            pg_canvas_rectangle(g, 0, y, sz.x, fh);
            pg_canvas_fill_stroke(g);
            pg_canvas_set_fill(g, cur_selected? conf.accent_fg: conf.accent);
            pg_canvas_show_string(g, f, p, y, items[i]);
            pg_canvas_set_fill(g, conf.fg);
        }
        else
            pg_canvas_show_string(g, f, p, y, items[i]);
        y += fh;
    }

    pg_canvas_set_stroke(g, conf.fg);
    pg_canvas_set_line_width(g, 1);
    pg_canvas_rectangle(g, 0.5f, 0.5f, sz.x - 1, sz.y - 1);
    pg_canvas_stroke(g);

    return selected;
}

int
main()
{
    PgWindow *main_win = pg_window_open(1280, 720, "test");
    PgWindowEvent *e;
    bool show = 0;
    int opt = 0;
    const char *families[256];
    int nfamilies = 0;
    const char *selected_family = "Arial Nova";

    for (const PgFamily *fam = pg_font_list(); fam->name; fam++)
        if (nfamilies < 256)
            families[nfamilies++] = strdup(fam->name);

    while ((e = pg_window_event_wait())) {
        pgg_begin(e);

        if (e->type == PG_EVENT_KEY_DOWN &&
            (!strcmp(e->key.key, "Ctrl+W") || !strcmp(e->key.key, "Escape")))
            break;

        show = pgg_checkbox("Show nonsense", show);
        if (show)
            pgg_text("Button Was Clicked!");

        if (pgg_button("OK"))
            puts("QQQ"),
            fflush(stdout);

        if (pgg_radio("OpenGL", opt == 0)) opt = 0, pg_window_queue_update(e->win);
        if (pgg_radio("DirectX", opt == 1)) opt = 1, pg_window_queue_update(e->win);
        if (pgg_radio("Vulkan", opt == 2)) opt = 2, pg_window_queue_update(e->win);

        pgg_text("This is text");
        pgg_text("More Test");


        selected_family = pgg_listbox(families, nfamilies, selected_family);

        pgg_end();
    }
}
