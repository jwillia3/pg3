#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3.h>
#include "internal.h"

#define ROUNDING    8.0f
#define SCROLLBAR   24.0f
#define PAD         8.0f
#define IPAD        (PAD * 2.0f)
#define FONT_SIZE   9.0f

static bool     theme_initialized;
static PgPaint  WINDOW;
static PgPaint  GUTTER;
static PgPaint  FACE;
static PgPaint  FACE_OVER;
static PgPaint  FACE_DOWN;
static PgPaint  TEXT_BG;
static PgPaint  TEXT_COLOR;
static PgPaint  CURSOR;
static PgPaint  SELECT;
static PgPaint  BORDER;

static Pg       *ctrl_canvas;
static PgGroup  *group;
static PgGroup  groups[32];
static PgFont   *_font;
static float    ctrlx;
static float    ctrly;
static float    ctrlsx;
static float    ctrlsy;
static unsigned uid;
static unsigned id;
static unsigned focused;
static unsigned active;


static
void
init_theme(void)
{
    if (theme_initialized)
        return;

    theme_initialized = true;
    WINDOW = pg_solid(PG_LCHAB, .9f, .0f, .0f, 1.0f);
    GUTTER = pg_solid(PG_LCHAB, .8f, .0f, .0f, 1.0f);
    FACE = pg_solid(PG_LCHAB, .75f, .0f, .0f, 1.0f);
    FACE_DOWN = pg_solid(PG_LCHAB, .5f, .0f, .0f, 1.0f);
    FACE_OVER = pg_solid(PG_LCHAB, .75f, .2f, .6f, 1.0f);
    TEXT_BG = pg_solid(PG_LCHAB, 1.0f, .0f, .0f, 1.0f);
    TEXT_COLOR = pg_solid(PG_LCHAB, .01f, .0f, .0f, 1.0f);
    CURSOR = pg_solid(PG_LCHAB, .0f, 1.0f, .0f, 1.0f);
    SELECT = pg_solid(PG_LCHAB, .75f, .2f, .6f, 1.0f);
    BORDER = pg_solid(PG_LCHAB, .015f, .0f, .0f, 1.0f);
}


float
pg_pad(void)
{
    return PAD;
}


static
void
free_prev_canvas(void)
{
    if (ctrl_canvas) {
        pg_free(ctrl_canvas);
        ctrl_canvas = 0;
    }
}


PgFont*
pg_font(void)
{
    init_theme();

    if (!_font) {
        _font = pg_find_font("FreeSans", 400, 0);
        pg_scale_font(_font,
            FONT_SIZE * 1.0f / 72.0f * pg_dpi().x,
            FONT_SIZE * 1.0f / 72.0f * pg_dpi().y);
    }

    return _font;
}


PgPt
pg_ctrl_at(void)
{
    return PgPt(ctrlx + group->abs_x, ctrly + group->abs_y);
}


PgPt
pg_ctrl_size(void)
{
    return PgPt(ctrlsx, ctrlsy);
}


Pg*
pg_ctrl_canvas(void)
{
    ctrl_canvas = pg_subcanvas(group->canvas, ctrlx, ctrly, ctrlsx, ctrlsy);
    return ctrl_canvas;
}


void
pg_group_clip(float x, float y, float sx, float sy)
{
    x += group->abs_x - group[-1].abs_x;
    y += group->abs_y - group[-1].abs_y;


    pg_free(group->canvas);
    group->canvas = pg_subcanvas(group[-1].canvas, x, y, sx, sy);
}


void
pg_set_group_pad(float x, float y)
{
    group->pad_x = x;
    group->pad_y = y;
}


unsigned
pg_ctrl_id(void)
{
    return id;
}


unsigned
pg_get_active(void)
{
    return active;
}


void
pg_set_active(unsigned id)
{
    active = id;
}


bool
pg_is_active(void)
{
    return pg_ctrl_id() == pg_get_active();
}


unsigned
pg_get_focused(void)
{
    return focused;
}


void
pg_set_focused(unsigned id)
{
    focused = id;
}


bool
pg_is_focused(void)
{
    return pg_ctrl_id() == pg_get_focused();
}


bool
pg_is_mouse_over(void)
{
    PgPt    a = pg_ctrl_at();
    PgPt    sz = pg_ctrl_size();
    PgPt    m = pg_mouse_at();
    return a.x <= m.x && a.y <= m.y && m.x < a.x + sz.x && m.y < a.y + sz.y;
}


Pg*
pg_ctrl(float sx, float sy)
{
    // Round size up to next pixel.
    sx = truncf(sx) + (sx - truncf(sx) != .0f? 1.0f: .0f);
    sy = truncf(sy) + (sy - truncf(sy) != .0f? 1.0f: .0f);

    id = uid++;
    ctrlx = group->x + group->pad_x;
    ctrly = group->y + group->pad_y;
    ctrlsx = sx;
    ctrlsy = sy;

    if (group->horiz)
        group->x = ctrlx + sx;
    else
        group->y = ctrly + sy;

    group->max_x = fmaxf(group->max_x, ctrlx + sx);
    group->max_y = fmaxf(group->max_y, ctrly + sy);

    // If no control is focused, take focus.
    if (!pg_get_focused())
        pg_set_focused(id);

    // Set canvas limited to the control area.
    free_prev_canvas();
    return pg_ctrl_canvas();
}


void
pg_set_group_cleanup(void subroutine(PgGroup *group), void *data)
{
    group->end = subroutine;
    group->data = data;
}


void
pg_group(bool horiz)
{
    if (group + 1 >= groups + sizeof groups / sizeof *groups)
        return;

    PgGroup   *old = group;
    Pg      *canvas = pg_subcanvas(old->canvas,
                          old->x,
                          old->y,
                          old->canvas->sx - old->x,
                          old->canvas->sy - old->y);

    group++;

    *group = (PgGroup) {
        .x = .0f,
        .y = .0f,
        .max_x = .0f,
        .max_y = .0f,
        .pad_x = old->pad_x,
        .pad_y = old->pad_y,
        .horiz = horiz,
        .canvas = canvas,
        .abs_x = old->abs_x + old->x,
        .abs_y = old->abs_y + old->y,
        .data = 0,
        .end = 0,
    };
}


void
pg_end_group(void)
{
    if (group <= groups)
        return;

    PgGroup     *prev = group;

    group--;

    if (group->horiz) {
        group->x += prev->max_x;
        group->max_x = fmaxf(group->max_x, group->x);
    }
    else {
        group->y += prev->max_y;
        group->max_y = fmaxf(group->max_y, group->y);
    }

    // Clean up the previous group.
    if (prev->end)
        prev->end(prev);

    pg_free(prev->canvas);
}


bool
pg_event(void)
{
    init_theme();
    free_prev_canvas();

    // Remove any groups that were not explicitly closed last frame.
    while (group > groups) {
        pg_end_group();
        group--;
    }

    // If no button is down, nothing is actively engaged with the mouse.
    if (!pg_mouse_buttons())
        pg_set_active(0);

    // Clear previous frame's key.
    pg_set_key(0);

    uid = 1;
    group = groups;

    Pg *root = pg_root_canvas();

    pg_reset_state(root);
    pg_set_clear(root, &WINDOW);

    Pg *canvas = pg_subcanvas(root, .0f, .0f, root->sx, root->sy);

    *group = (PgGroup) {
        .x = .0f,
        .y = .0f,
        .max_x = .0f,
        .max_y = .0f,
        .pad_x = PAD,
        .pad_y = PAD,
        .horiz = false,
        .canvas = canvas,
        .data = 0,
        .end = 0,
    };

    return pg_wait();
}


bool
pg_should_activate(void)
{
    if (pg_is_mouse_over()) {

        if (pg_mouse_buttons() == 1 && !pg_get_active()) {
            pg_set_active(pg_ctrl_id());
            pg_set_focused(pg_ctrl_id());
            pg_redraw();
        }

        if (pg_is_active()) {
            if (pg_mouse_buttons() == 0)
                return true;
        }
    }

    if (pg_is_focused()) {
        if (pg_key() == PG_KEY_ENTER || pg_key() == ' ')
            return true;
    }

    return false;
}


bool
pg_checkbox(const char *text, bool *checked)
{
    PgFont  *font = pg_font();
    float   sy = pg_font_height(font);
    float   sx = pg_measure_string(font, text) + 2 * IPAD;
    Pg      *g = pg_ctrl(sx, sy);

    if (pg_should_activate())
        *checked = !*checked;


    pg_set_fill(g, &TEXT_BG);
    pg_set_stroke(g, &BORDER);
    pg_rectangle(g, 1.0f, 1.0f, IPAD - 2.0f, sy - 2.0f);
    pg_fill_stroke(g);


    if (*checked) {
        pg_set_fill(g, &TEXT_COLOR);
        pg_set_stroke(g, &TEXT_COLOR);
        pg_set_line_width(g, 2.0f);
        pg_move(g, IPAD * .2f, IPAD * .6f);
        pg_rline(g, IPAD * .1f, IPAD * .2f);
        pg_rline(g, IPAD * .5f, IPAD * -0.5f);
        pg_stroke(g);
        pg_set_line_width(g, 1.0f);
    }

    pg_set_fill(g, &TEXT_COLOR);
    pg_string_path(g, font, 1.5f * IPAD, .0f, text);
    pg_fill(g);

    return *checked;
}


bool
pg_button(const char *text)
{
    PgFont  *font = pg_font();
    float   sy = pg_font_height(font) + 2 * IPAD;
    float   sx = pg_measure_string(font, text) + 2 * IPAD;
    Pg      *g = pg_ctrl(sx, sy);
    bool    clicked = pg_should_activate();

    pg_set_fill(g,
        pg_is_active() && pg_mouse_buttons() == 1 ? &FACE_DOWN:
        pg_is_mouse_over()? &FACE_OVER:
        &FACE);
    pg_set_stroke(g, &BORDER);
    pg_rounded(g, 1.0f, 1.0f, sx - 2.0f, sy - 2.0f, ROUNDING, ROUNDING);
    pg_fill_stroke(g);

    pg_set_fill(g, &TEXT_COLOR);
    pg_string_path(g, font, IPAD, IPAD, text);
    pg_fill(g);

    return clicked;
}


void
pg_label(const char *text)
{
    PgFont  *font = pg_font();
    float   sy = pg_font_height(font);
    float   sx = pg_measure_string(font, text);
    Pg      *g = pg_ctrl(sx, sy);

    pg_set_fill(g, &TEXT_COLOR);
    pg_string_path(g, font, .0f, .0f, text);
    pg_fill(g);
}


static
void
slider_key(float total, float *val)
{
    float   step = 10.0f / total;

    if (pg_is_focused() && pg_key()) {
        switch (pg_key()) {
        case PG_KEY_UP:
        case PG_KEY_LEFT:
            *val -= step;
            break;

        case PG_KEY_DOWN:
        case PG_KEY_RIGHT:
            *val += step;
            break;

        case PG_KEY_PAGE_UP:
            *val -= .1f;
            break;

        case PG_KEY_PAGE_DOWN:
            *val += .1f;
            break;
        }
    }

    if (pg_is_mouse_over() && pg_mouse_wheel() != .0f)
        *val += step * pg_mouse_wheel();
}


void
pg_hslider(float width, float *val)
{
    if (width <= 0.0f)
        return;

    if (!val)
        return;

    float   vx = IPAD;
    float   vy = IPAD;
    float   sx = width;
    float   sy = vy;
    Pg      *g = pg_ctrl(sx, sy);

    if (pg_is_mouse_over()) {
        if (pg_mouse_buttons() == 1) {
            pg_set_active(pg_ctrl_id());
            pg_set_focused(pg_ctrl_id());
            float offset = pg_mouse_at().x - pg_ctrl_at().x;
            *val = offset / sx;
        }
    }

    slider_key(width, val);

    *val = fmaxf(.0f, fminf(*val, 1.0f));

    pg_rounded(g, .0f, .0f, sx, sy, ROUNDING, ROUNDING);
    pg_set_fill(g, &GUTTER);
    pg_set_stroke(g, &BORDER);
    pg_fill_stroke(g);

    float   x = fminf(*val * width, sx - vx);
    pg_rounded(g, x, .0f, vx, vy, ROUNDING, ROUNDING);
    pg_set_fill(g, pg_is_mouse_over()? &FACE_OVER: &FACE);
    pg_set_stroke(g, &BORDER);
    pg_fill_stroke(g);
}


void
pg_vslider(float height, float *val)
{
    if (height <= 0.0f)
        return;

    if (!val)
        return;

    float   vx = SCROLLBAR;
    float   vy = SCROLLBAR;
    float   sx = vx;
    float   sy = height;
    Pg      *g = pg_ctrl(sx, sy);

    if (pg_is_mouse_over()) {
        if (pg_mouse_buttons() == 1) {
            pg_set_active(pg_ctrl_id());
            pg_set_focused(pg_ctrl_id());
            float offset = pg_mouse_at().y - pg_ctrl_at().y;
            *val = offset / sy;
        }
    }

    slider_key(height, val);

    *val = fmaxf(.0f, fminf(*val, 1.0f));

    pg_rounded(g, 1.0f, 1.0f, sx - 2.0f, sy - 2.0f, ROUNDING, ROUNDING);
    pg_set_fill(g, &GUTTER);
    pg_set_stroke(g, &BORDER);
    pg_fill_stroke(g);

    float   y = fminf(*val * height, sy - vy);
    pg_rounded(g, 1.0f, y + 1.0f, vx - 2.0f, vy - 2.0f, ROUNDING, ROUNDING);
    pg_set_fill(g, pg_is_mouse_over()? &FACE_OVER: &FACE);
    pg_set_stroke(g, &BORDER);
    pg_fill_stroke(g);
}


bool
pg_item(float sx, const char *text, bool selected)
{
    PgFont  *font = pg_font();
    float   sy = pg_font_height(font);
    Pg      *g = pg_ctrl(sx, sy);

    if (pg_is_mouse_over() || selected) {
        pg_set_clear(g, &SELECT);
        pg_clear(g);
    }

    bool clicked = pg_should_activate();

    pg_set_fill(g, &TEXT_COLOR);
    pg_string_path(g, font, PAD, .0f, text);
    pg_fill(g);

    return clicked;
}


bool
pg_dropdown(float sx, const char *text, bool *open)
{
    PgFont  *font = pg_font();
    float   sy = pg_font_height(font) + 2.0f * PAD;
    Pg      *g = pg_ctrl(sx, sy);

    if (pg_should_activate())
        *open = !*open;

    pg_set_fill(g,
        pg_is_active()? &FACE_DOWN:
        pg_is_mouse_over()? &FACE_OVER:
        &FACE);
    pg_set_stroke(g, &BORDER);
    pg_rounded(g, 1.0f, 1.0f, sx - 2.0f, sy - 2.0f, ROUNDING, ROUNDING);
    pg_fill_stroke(g);

    pg_set_fill(g, &TEXT_COLOR);
    pg_string_path(g, font, PAD, PAD, text);
    pg_fill(g);

    return *open;
}


static
void
end_vscroll(PgGroup *group)
{
    float   sx = group->canvas->sx;
    float   sy = group->canvas->sy;
    float   *vscroll = group->data;

    pg_reset_state(group->canvas);
    pg_rounded(group->canvas,
               PAD + 1.0f,
               1.0f,
               sx - 2.0f - PAD,
               sy - 2.0f,
               ROUNDING,
               ROUNDING);
    pg_set_stroke(group->canvas, &BORDER);
    pg_stroke(group->canvas);

    pg_set_group_pad(.0f, .0f);
    pg_vslider(sy, vscroll);
    pg_end_group();
}


void
pg_vscroll(float sx, float sy, float total_y, float *vscroll)
{
    pg_group(true);

    pg_group(false);
    pg_group_clip(0.0f, 0.0f, sx, sy);
    pg_set_group_cleanup(end_vscroll, vscroll);

    pg_set_group_pad(PAD, PAD);
    group->y -= *vscroll * (total_y - sy);
}



static unsigned textbox_id;
static size_t cursor;


static
size_t
bytes_back(const char *text, size_t at, size_t len)
{
    if (at == 0)
        return 0;

    const char *p = pg_utf8_start(text + at - 1, text, text + len);
    return (size_t) ((text + at) - p);
}


static
size_t
bytes_forward(const char *text, size_t at, size_t len)
{
    if (at == len)
        return 0;

    const char *next = text + at;

    pg_read_utf8(&next, text + len);

    return (size_t) (next - (text + at));
}

static
size_t
delete(char *text, size_t at, size_t nbytes, size_t len)
{
    memmove(text + at, text + at + nbytes, len - at + 1);
    return nbytes;
}


static
size_t
insert(char *text, size_t at, uint32_t codepoint, size_t len, size_t limit)
{
    size_t  nbytes = pg_utf8_nbytes(codepoint);

    if (len + nbytes >= limit)
        return 0;

    memmove(text + at + nbytes, text + at, len - at + 1);
    pg_write_utf8(text + at, text + at + nbytes, codepoint);
    return nbytes;

}



static
bool
textbox_keyboard(char *text, size_t len, size_t limit)
{

    // Graphical character.
    if (pg_key() > 0 && !(pg_mod_keys() & (PG_MOD_CTRL | PG_MOD_ALT))) {
        cursor += insert(text, cursor, (uint32_t) pg_key(), len, limit);
        return false;
    }


    bool ctrl = pg_mod_keys() == PG_MOD_CTRL;


    // Functional key.
    switch (pg_key()) {

    case 'A':
    case 'a':
        if (ctrl)
            cursor = 0;
        break;

    case 'E':
    case 'e':
        if (ctrl)
            cursor = len;
        break;

    case 'K':
    case 'k':
        if (ctrl)
            delete(text, cursor, len - cursor, len);
        break;

    case 'U':
    case 'u':
        if (ctrl) {
            delete(text, 0, cursor, len);
            cursor = 0;
        }
        break;

    case 'M':
    case 'm':
        if (ctrl)
            return true;
        break;

    case PG_KEY_ENTER:
        if (pg_mod_keys() == 0)
            return true;
        break;

    case PG_KEY_LEFT:
        if (cursor > 0) {
            cursor -= bytes_back(text, cursor, len);
        }
        break;

    case PG_KEY_RIGHT:
        if (cursor < len) {
            cursor += bytes_forward(text, cursor, len);
        }
        break;

    case PG_KEY_HOME:
        cursor = 0;
        break;

    case PG_KEY_END:
        cursor = len;
        break;

    case PG_KEY_DELETE:
        {
            size_t n = bytes_forward(text, cursor, len);
            delete(text, cursor, n, len);
        }
        break;

    case PG_KEY_BACKSPACE:
        if (cursor > 0) {
            size_t n = bytes_back(text, cursor, len);
            cursor -= n;
            delete(text, cursor, n, len);
        }
        break;
    }

    return false;
}


bool
pg_textbox(float width, char *text, size_t limit)
{

    // Reset state if the focused box has changed.
    if (pg_is_active() && textbox_id != pg_ctrl_id()) {
        cursor = 0;
        textbox_id = pg_ctrl_id();
    }

    PgFont  *font = pg_font();
    size_t  len = strlen(text);
    float   text_height = pg_font_height(font);
    float   sx = width;
    float   sy = text_height + PAD * 2.0f;
    Pg      *g = pg_ctrl(sx, sy);
    bool    entered = false;

    pg_should_activate();

    if (pg_is_mouse_over() && pg_mouse_buttons() == 1) {
        float x = pg_mouse_at().x - pg_ctrl_at().x - PAD;
        unsigned index = pg_fit_string(font, text, x);
        cursor = index;
    }

    if (pg_is_focused()) {
        entered = textbox_keyboard(text, len, limit);
        len = strlen(text);
    }

    pg_set_fill(g, &TEXT_BG);
    pg_set_stroke(g, &BORDER);
    pg_rounded(g, 1.0f, 1.0f, sx - 2.0f, sy - 2.0f, ROUNDING, ROUNDING);
    pg_fill_stroke(g);

    pg_set_fill(g, &TEXT_COLOR);
    pg_string_path(g, font, PAD, PAD, text);
    pg_fill(g);

    if (pg_is_focused()) {
        float cx = pg_measure_chars(font, text, cursor);
        pg_set_fill(g, &CURSOR);
        pg_rectangle(g, PAD + cx, PAD, 2.0f, text_height);
        pg_fill(g);
    }

    return entered;
}
