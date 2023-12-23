#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3/pg.h>
#include <pg3/pg-box.h>

static const pgb_type_t type;


static bool on_mouse_down(pgb_t *box, void *etc);
static bool on_mouse_up(pgb_t *box, void *etc);
static bool on_mouse_move(pgb_t *box, void *etc);


pgb_t*
pgb_button(const char *arg, ...)
{
    pgb_t *box = pgb_box(&type,
        "text", "",
        "enabled", "true",
        "pressing", "false",
        "ipad", "12",

        "can-focus", "true",
        "opaque", "true",
        "on-mouse-move", on_mouse_move,
        "on-mouse-up", on_mouse_up,
        "on-mouse-down", on_mouse_down,
        "on-click", NULL,
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
    if (box->children)
        return pgb_default_measure(box);

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
    if (box->children) {
        pgb_default_draw(box, g);
        return;
    }

    PgFont      *f = pgb_font();
    const char  *text = pgb_prop_get(box, "text");
    float       th = pg_font_get_height(f);
    float       tw = pg_font_measure_string(f, text);
    PgPt        size = box->size;
    bool        hover = pgb_is_hover(box);
    bool        focus = pgb_is_focus(box);
    bool        pressing = pgb_prop_get_bool(box, "pressing");
    bool        disabled = !pgb_prop_get_bool(box, "enabled");

    const char  *face_color = pressing? "ui-fg": "ui-face";
    const char  *line_color = pressing? "ui-fg":
                              focus? "ui-accent":
                              hover && !disabled? "ui-accent":
                              "ui-fg";
    const char  *text_color = pressing? "ui-bg":
                              disabled? "ui-dim":
                              hover? "ui-accent":
                              "ui-fg";

    pg_canvas_set_fill(g, pg_paint_from_name(face_color));
    pg_canvas_set_stroke(g, pg_paint_from_name(line_color));
    pg_canvas_set_line_width(g, 2.0f);
    pg_canvas_rounded_rectangle(g, 1, 1, size.x - 2, size.y - 2, 8);
    pg_canvas_fill_stroke(g);

    pg_canvas_set_fill(g, pg_paint_from_name(text_color));
    pg_canvas_show_string(g, f, (size.x - tw) * .5f, (size.y - th) * .5f, text);
}


static bool
on_mouse_move(pgb_t *box, void *etc)
{
    (void) etc;
    bool pressing = pgb_prop_get_bool(box, "enabled") && pgb_is_capture(box) && pgb_is_hover(box);
    if (pressing != pgb_prop_get_bool(box, "pressing"))
        pgb_prop_set_bool(box, "pressing", pressing),
        pgb_update(box);
    return false;
}


static bool
on_mouse_down(pgb_t *box, void *etc)
{
    const char *button = ((pgb_mouse_down_t*) etc)->button;
    if (strcmp("LeftButton", button)) return false;

    if (pgb_prop_get_bool(box, "enabled")) {
        pgb_set_capture(box, box),
        pgb_prop_set_bool(box, "pressing", true);
        pgb_update(box);
    }
    return false;
}


static bool
on_mouse_up(pgb_t *box, void *etc)
{
    const char *button = ((pgb_mouse_up_t*) etc)->button;
    if (strcmp("LeftButton", button)) return false;

    /*
        On releasing the mouse button, the press is always done.
        If this button is capturing the mouse, it was received the
        initial mouse down. Only click if the mouse is STILL over
        that button.
     */

    pgb_prop_set_bool(box, "pressing", false);
    pgb_update(box);

    if (pgb_is_capture(box)) {
        pgb_set_capture(box, NULL);

        if (pgb_prop_get_bool(box, "enabled") && pgb_is_hover(box))
            pgb_event(box, "click", NULL);
    }
    return false;
}


static const pgb_type_t
type = {
    "button",
    .measure = measure,
    .pack = pgb_default_pack,
    .draw = draw,
};
