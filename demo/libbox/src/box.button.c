#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-box.h>
#include <pg-internal-box.h>

typedef struct {
    bool    disabled;
    bool    pressed;
    bool    hovered;
} Info;


static Info*
get_info(const PgBox *box)
{
    return pg_box_get_sys(box);
}


static void
paint(PgBox *box, Pg *g)
{
    const float     LINE = 2.0f;
    const float     RADIUS = 8.0f;
    Info            *info = get_info(box);
    const PgPaint   *bg = pg_paint_from_name(info->disabled? "clear":
                                             info->pressed? "ui-dim":
                                             info->hovered? "clear":
                                             "clear");
    const PgPaint   *fg = pg_paint_from_name(info->disabled? "ui-dim":
                                             info->pressed? "ui-fg":
                                             info->hovered? "ui-accent":
                                             "ui-fg");

    pg_canvas_set_fill(g, bg);
    pg_canvas_set_stroke(g, fg);
    pg_canvas_set_line_width(g, LINE);
    pg_canvas_rounded_rectangle(g,
        .5f * LINE,
        .5f * LINE,
        pg_box_get_size(box).x - LINE,
        pg_box_get_size(box).y - LINE,
        RADIUS,
        RADIUS);
    pg_canvas_fill_stroke(g);

    pg_box_default_paint(box, g);
}


static void
hovered(PgBox *box, bool is_hovered)
{
    get_info(box)->hovered = is_hovered;
    pg_box_update(box);
}


PgBox*
pg_box_button(PgBoxFlags flags)
{
    static const PgBoxType type = {
        .type = "BUTTON",
        .paint = paint,
        .hovered = hovered,
    };
    return pg_box_set_sys(pg_box_new(&type, flags),
                          pgnew(Info,
                              .disabled = false,
                              .pressed = false));
}


PgBox*
pg_box_buttoned(PgBoxFlags flags, PgBox **children)
{
    return pg_box_added(pg_box_button(flags), children);
}


PgBox*
pg_box_button_text(PgBoxFlags flags, const char *text)
{
    return pg_box_buttoned(flags, (PgBox*[]) {
        pg_box_fill(),
        pg_box_text(PG_BOX_FLAG_MINOR_CENTER, text),
        pg_box_fill(),
        NULL });
}
