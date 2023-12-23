#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-box.h>

static const pgb_type_t type;


pgb_t*
pgb_group(const char *arg, ...)
{
    pgb_t *box = pgb_box(&type,
        "ipad", "8",
        "background", "clear",
        "border-color", "ui-fg",
        "border-radius", "16",
        "border-width", "0",
        NULL);
    va_list ap;
    va_start(ap, arg);
    pgb_props_set_var(box, arg, ap);
    va_end(ap);
    return box;
}


static void
draw(pgb_t *box, Pg *g)
{
    float   bw = pgb_prop_get_float(box, "border-width");
    float   br = pgb_prop_get_float(box, "border-radius");
    PgPt    sz = box->size;
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
    pgb_default_draw(box, g);
}


static const pgb_type_t
type = {
    "group",
    .measure = pgb_default_measure,
    .draw = draw,
};
