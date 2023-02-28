#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

const float LINE = 1.5f;
const float ROUND = 4.0f;
const float DEF_LENGTH = 128;
const float DEF_WIDTH = 16;
const float MIN_TACK = 12;


typedef struct Info Info;
struct Info {
    float   value;
    bool    horiz;
    bool    dragging;
    void    (*scrolled)(Pgb *box, float value);
};


static
void
change(Pgb *box, float value)
{
    Info *info = pgb_get_sys(box);
    info->value = value = fmaxf(0.0f, fminf(1.0f, value));
    pgb_dirty(box);
    if (info->scrolled)
        info->scrolled(box, value);
}

static
void
draw(Pgb *box, Pg *g)
{
    Info *info = pgb_get_sys(box);

    PgPaint *line_colour = pgb_color(pgb_is_hovered(box)? PGB_ACCENT_COLOR:
                                     pgb_is_focused(box)? PGB_ACCENT_COLOR:
                                     PGB_DARK_COLOR);
    PgPaint *gutter = pgb_color(PGB_LIGHT_COLOR);
    PgPaint *face = pgb_color(pgb_is_focused(box)? PGB_ACCENT_COLOR:
                              PGB_LIGHT_COLOR);

    pg_rounded(g,
               LINE / 2,
               LINE / 2,
               box->sx - LINE,
               box->sy - LINE,
               ROUND,
               ROUND);
    pg_set_fill(g, gutter);
    pg_set_stroke(g, line_colour);
    pg_fill_stroke(g);

    float pos = info->value * ((info->horiz? box->sx: box->sy) - MIN_TACK);

    if (info->horiz)
        pg_rounded(g, pos, 0.0f, MIN_TACK, box->sy, ROUND, ROUND);
    else
        pg_rounded(g, 0.0f, pos, box->sx, MIN_TACK, ROUND, ROUND);
    pg_set_fill(g, face);
    pg_fill_stroke(g);
}


static
void
mouse_down(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) box, (void) x, (void) y, (void) button, (void) mods;
    Info *info = pgb_get_sys(box);
    pgb_set_focused(box);
    if (button == 0 && !mods)
        info->dragging = true;
}


static
void
mouse_up(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) box, (void) x, (void) y, (void) button, (void) mods;
    Info *info = pgb_get_sys(box);
    info->dragging = false;
}


static
void
mouse_move(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) button, (void) mods;
    Info *info = pgb_get_sys(box);
    if (info->dragging)
        change(box, info->horiz? x / (box->sx - MIN_TACK):
                                y / (box->sy - MIN_TACK));
}


static
void
key_down(Pgb *box, PgKey key, PgMods mods)
{
    Info *info = pgb_get_sys(box);
    float step = 0.1f;

    if (info->horiz && key == PG_KEY_LEFT && !mods)
        change(box, info->value - step);
    else if (info->horiz && key == PG_KEY_RIGHT && !mods)
        change(box, info->value + step);
    else if (!info->horiz && key == PG_KEY_UP && !mods)
        change(box, info->value - step);
    else if (!info->horiz && key == PG_KEY_DOWN && !mods)
        change(box, info->value + step);
    else if ((key == ' ' || key == PG_KEY_ENTER) && !mods)
        change(box, info->value + 1);
    else
        return;

    pgb_dirty(box);
}


static
void
hover(Pgb *box, bool over)
{
    Info *info = pgb_get_sys(box);
    if (!over)
        info->dragging = false;
    pgb_dirty(box);
}


static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = {
        .draw = draw,
        .mouse_down = mouse_down,
        .mouse_up = mouse_up,
        .mouse_move = mouse_move,
        .key_down = key_down,
        .hover = hover,
    };
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_scrollbar(bool horiz,
              float value,
              void (*scrolled)(Pgb *box, float value),
              PgbFlags flags)
{
    Info *info = new(Info,
                   .value = value,
                   .horiz = horiz,
                   .scrolled = scrolled);
    float cx = horiz? DEF_LENGTH: DEF_WIDTH;
    float cy = horiz? DEF_WIDTH: DEF_LENGTH;
    return pgb_set_sys(pgb_box(cx, cy, flags, methods()), info);
}
