#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-box.h>

static const pgb_type_t listbox_type;
static const pgb_type_t listbox_item_type;


pgb_t*
pgb_listbox(const char *arg, ...)
{
    pgb_t *box = pgb_box(&listbox_type,
        "foreground", "ui-fg",
        "vertical", "true",
        "item-spacing", "0",
        NULL);
    va_list ap;
    va_start(ap, arg);
    pgb_props_set_var(box, arg, ap);
    va_end(ap);
    return box;
}


static PgPt
item_measure(const pgb_t *box)
{
    PgFont      *f = pgb_font();
    const char  *text = pgb_prop_get(box, "text");
    float       th = pg_font_get_height(f);
    float       tw = pg_font_measure_string(f, text);
    float       ipad = pgb_prop_get_float(box, "ipad");
    return pgpt(tw + ipad * 2, th + ipad * 2);
}


static void
item_draw(pgb_t *box, Pg *g)
{
    PgFont      *f = pgb_font();
    const char  *text = pgb_prop_get(box, "text");
    bool        hover = pgb_is_hover(box);
    // const char  *bg = hover? "ui-accent": "ui-bg";
    const char  *fg = hover? "ui-accent": "ui-fg";
    float       ipad = pgb_prop_get_float(box, "ipad");

    pg_canvas_set_clear(g, pg_paint_from_name("ui-bg"));
    pg_canvas_clear(g);

    pg_canvas_set_fill(g, pg_paint_from_name(fg));
    pg_canvas_show_string(g, f, ipad, ipad, text);
}


pgb_t*
pgb_listbox_item(const char *arg, ...)
{
    pgb_t *box = pgb_box(&listbox_item_type,
        "minor", "fill",
        "can-focus", "true",
        NULL);
    va_list ap;
    va_start(ap, arg);
    pgb_props_set_var(box, arg, ap);
    va_end(ap);
    return box;
}


static const pgb_type_t
listbox_type = {
    "listbox",
    .draw = NULL,
};

static const pgb_type_t
listbox_item_type = {
    "listbox-item",
    .draw = item_draw,
    .measure = item_measure,
};
