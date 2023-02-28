#include <math.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>

static const float LINE = 1.5f;
static const float ROUND = 4;


PgbColor
pgb_get_border_color(Pgb *box)
{
    return (intptr_t) pgb_get_sys(box);
}


void
pgb_set_border_color(Pgb *box, PgbColor color)
{
    pgb_set_sys(box, (void*) (intptr_t) color);
}


static
void
draw(Pgb *box, Pg *g)
{
    PgPaint *color = pgb_color(pgb_get_border_color(box));
    pg_rounded(g,
               LINE / 2.0f,
               LINE / 2.0f,
               box->sx - LINE,
               box->sy - LINE,
               ROUND,
               ROUND);
    pg_set_fill(g, pgb_color(PGB_LIGHT_COLOR));
    pg_set_line_width(g, LINE);
    pg_set_stroke(g, color);
    pg_fill_stroke(g);

    pgb_default_draw(box, g);
}


static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = { .draw = draw };
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_border(Pgb *content, PgbFlags flags)
{
    Pgb *box = pgb_add(pgb_box(0, 0, flags | PGB_OPAD, methods()), content);
    pgb_set_border_color(box, PGB_MID_COLOR);
    return box;
}


Pgb*
pgb_bordered(PgbFlags flags, Pgb **content)
{
    return pgb_add_list(pgb_border(0, flags), content);
}
