#include <pg.h>
#include <pg-box.h>
#include <pg-internal-box.h>

static void
paint(PgBox *box, Pg *g)
{
    const float LINE = 2.0f;
    const float RADIUS = 8.0f;

    pg_box_default_paint(box, g);

    pg_canvas_state_reset(g);
    pg_canvas_set_stroke(g, pg_paint_from_name("ui-fg"));
    pg_canvas_set_line_width(g, LINE);
    pg_canvas_rounded_rectangle(g,
        .5f * LINE,
        .5f * LINE,
        pg_box_get_size(box).x - LINE,
        pg_box_get_size(box).y - LINE,
        RADIUS,
        RADIUS);
    pg_canvas_stroke(g);
}


PgBox*
pg_box_new_border(PgBoxFlags flags)
{
    const static PgBoxType type = {
        .paint = paint,
    };
    return pg_box_new(&type, flags);
}


PgBox*
pg_box_new_vborder(PgBoxFlags flags)
{
    return pg_box_new_border(flags | PG_BOX_FLAG_VERT);
}


PgBox*
pg_box_bordered(PgBoxFlags flags, PgBox **children)
{
    return pg_box_added(pg_box_new_border(flags), children);
}


PgBox*
pg_box_vbordered(PgBoxFlags flags, PgBox **children)
{
    return pg_box_added(pg_box_new_vborder(flags), children);
}
