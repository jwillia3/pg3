#include <stdlib.h>
#include <string.h>
#include <pg3/pg.h>
#include <pg3/pg-box.h>

static const pgb_type_t type;


static PgFont*
_font(const pgb_t *box)
{
    const char *style = pgb_prop_get(box, "style");
    return  !strcmp("normal", style)? pgb_font():
            !strcmp("title", style)? pgb_title_font():
            !strcmp("subtitle", style)? pgb_subtitle_font():
            !strcmp("monospace", style)? pgb_monospace_font():
            pgb_font();
}


pgb_t*
pgb_label(const char *arg, ...)
{
    pgb_t *box = pgb_box(&type,
        "text", "",
        "foreground", "ui-fg",
        "style", "normal",
        NULL);
    va_list ap;
    va_start(ap, arg);
    pgb_props_set_var(box, arg, ap);
    va_end(ap);
    return box;
}


static PgPt
measure(const pgb_t *box)
{
    PgFont      *f = _font(box);
    const char  *text = pgb_prop_get(box, "text");
    float       th = pg_font_get_height(f);
    float       tw = pg_font_measure_string(f, text);
    float       ipad = pgb_prop_get_float(box, "ipad");
    return pgpt(tw + ipad * 2, th + ipad * 2);
}


static void
draw(pgb_t *box, Pg *g)
{
    PgFont      *f = _font(box);
    const char  *text = pgb_prop_get(box, "text");
    const char  *fg = pgb_prop_get(box, "foreground");
    float       ipad = pgb_prop_get_float(box, "ipad");

    pg_canvas_set_fill(g, pg_paint_from_name(fg));
    pg_canvas_show_string(g, f, ipad, ipad, text);
}


static const pgb_type_t
type = {
    "label",
    .measure = measure,
    .draw = draw,
};
