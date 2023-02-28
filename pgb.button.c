#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

static const float PX = 24.0f;
static const float PY = 12.0f;
static const float LINE = 1.5f;
static const float ROUND = 8.0f;

typedef struct Btn Btn;

struct Btn {
    char    *text;
    bool    pressed;
    bool    disabled;
    void    (*clicked)(Pgb *box);
    void    (*hover)(Pgb *box, bool over);
    void    (*focus)(Pgb *box, bool on);
};


bool
pgb_is_button_enabled(Pgb *box)
{
    Btn *btn = pgb_get_sys(box);
    return btn? !btn->disabled: false;
}


Pgb*
pgb_enable_button(Pgb *box, bool enabled)
{
    Btn *btn = pgb_get_sys(box);
    if (!btn) return box;
    btn->disabled = !enabled;
    pgb_dirty(box);
    return box;
}


Pgb*
pgb_set_button_text(Pgb *box, const char *text)
{
    Btn *btn = pgb_get_sys(box);
    if (!btn || !text) return box;
    free(btn->text);
    btn->text = strdup(text);
    pgb_dirty(box);
    return box;
}


const char *
pgb_get_button_text(Pgb *box)
{
    Btn *btn = pgb_get_sys(box);
    return btn? btn->text: 0;
}


void
pgb_set_on_button_hover(Pgb *box, void (*hover)(Pgb *box, bool over))
{
    Btn *btn = pgb_get_sys(box);
    if (!btn) return;
    btn->hover = hover;
}


void
pgb_set_on_button_focus(Pgb *box, void (*focus)(Pgb *box, bool on))
{
    Btn *btn = pgb_get_sys(box);
    if (!btn) return;
    btn->focus = focus;
}


static
void
hover(Pgb *box, bool over)
{
    Btn *btn = pgb_get_sys(box);

    if (!over) {
        /*
            If we move the mouse off of the button, we don't
            want the mouse up to count as a click.
         */
        btn->pressed = false;
        pgb_dirty(box);
    }
    else if (!btn->disabled)
        pgb_dirty(box);

    if (btn->hover)
        btn->hover(box, over);
}


static
void
focus(Pgb *box, bool on)
{
    Btn *btn = pgb_get_sys(box);
    if (btn->focus)
        btn->focus(box, on);
}


static
void
mouse_down(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) x, (void) y, (void) mods;

    Btn *btn = pgb_get_sys(box);

    if (button == 0 && !btn->disabled) {
        btn->pressed = true;
        pgb_set_focused(box);
        pgb_dirty(box);
    }
}


static
void
key_down(Pgb *box, PgKey key, PgMods mods)
{
    (void) key;

    if (!mods && (key == PG_KEY_ENTER || key == ' '))
        pgb_on_click(box, 0.0f, 0.0f, 0, 0);
    else
        pgb_defaults().key_down(box, key, mods);
}



static
void
mouse_up(Pgb *box, float x, float y, int button, PgMods mods)
{
    Btn *btn = pgb_get_sys(box);

    if (button == 0 && !btn->disabled) {
        if (pgb_is_focused(box) && btn->pressed)
            /*
                We moused down on the button (to get focus) and we
                complete the click with the mouse up. If we don't
                have focus, we clicked down on something else.
            */
            pgb_on_click(box, x, y, button, mods);

        btn->pressed = false;
        pgb_dirty(box);
    }
}


static
void
click(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) x, (void) y, (void) button, (void) mods;

    Btn *btn = pgb_get_sys(box);

    if (!btn->disabled && btn->clicked)
        btn->clicked(box);
}


/*
    Buttons are always opaque with hit detection.
    If we have children instead of just text, we want
    this to hit here and not on the children.
    For text, this is normal behaviour.
 */
static
Pgb*
hit(Pgb *box, float x, float y, PgPt *adjusted)
{
    if (!pgb_point_in(box, x, y))
        return 0;

    if (adjusted)
        *adjusted = PgPt(x - box->x, y - box->y);
    return box;
}


static
void
draw(Pgb *box, Pg *g)
{
    Btn *btn = pgb_get_sys(box);

    if (box->child) {
        pgb_default_draw(box, g);
        return;
    }

    const char *text = btn->text;
    PgFont *font = pgb_font(PGB_DEFAULT_FONT);
    PgPaint *face_colour = btn->disabled? pgb_color(PGB_LIGHT_COLOR):
                           btn->pressed? pgb_color(PGB_ACCENT_COLOR):
                           pgb_color(PGB_LIGHT_COLOR);
    PgPaint *text_colour = btn->disabled? pgb_color(PGB_MID_COLOR):
                           btn->pressed? pgb_color(PGB_LIGHT_COLOR):
                           pgb_is_focused(box)? pgb_color(PGB_ACCENT_COLOR):
                           pgb_is_hovered(box)? pgb_color(PGB_ACCENT_COLOR):
                           pgb_color(PGB_DARK_COLOR);
    PgPaint *line_colour = btn->disabled? pgb_color(PGB_MID_COLOR):
                           btn->pressed? pgb_color(PGB_ACCENT_COLOR):
                           pgb_is_focused(box)? pgb_color(PGB_ACCENT_COLOR):
                           pgb_is_hovered(box)? pgb_color(PGB_ACCENT_COLOR):
                           pgb_color(PGB_DARK_COLOR);

    pg_rounded(g,
               LINE / 2,
               LINE / 2,
               box->sx - LINE,
               box->sy - LINE,
               ROUND,
               ROUND);
    pg_set_fill(g, face_colour);
    pg_set_stroke(g, line_colour);
    pg_set_line_width(g, LINE);
    pg_fill_stroke(g);

    float cx = pg_measure_string(font, text);
    float cy = pg_font_height(font);
    pg_string_path(g,
                   font,
                   (box->sx - cx) * .5f,
                   (box->sy - cy) * .5f,
                   text);
    pg_set_fill(g, text_colour);
    pg_fill(g);
}


static
void
_free(Pgb *box)
{
    Btn *button = pgb_get_sys(box);
    free(button->text);
    free(button);
}


static
void
autosize(Pgb *box)
{
    Btn *button = pgb_get_sys(box);
    const char *text = button->text;
    if (box->child)
        pgb_default_autosize(box);
    else {
        PgFont *font = pgb_font(PGB_DEFAULT_FONT);
        box->cx = ceilf(pg_measure_string(font, text) + PX * 2.0f);
        box->cy = ceilf(pg_font_height(font) + PY * 2.0f);
    }
}


static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = {
        .draw = draw,
        .free = _free,
        .autosize = autosize,
        .hover = hover,
        .focus = focus,
        .key_down = key_down,
        .mouse_up = mouse_up,
        .mouse_down = mouse_down,
        .click = click,
        .hit = hit,
    };

    if (done) return &methods;

    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_button(const char *text, void (*clicked)(Pgb *box), PgbFlags flags)
{
    return pgb_set_sys(
        pgb_box(0.0f, 0.0f, flags, methods()),
        new(Btn,
            .text = text? strdup(text): strdup(""),
            .clicked = clicked));
}
