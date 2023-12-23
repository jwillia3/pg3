#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3/pg.h>
#include <pg3/pg-box.h>


static bool
within(const pgb_t *box, float x, float y)
{
    return  box->at.x <= x &&
            box->at.y <= y &&
            x < box->at.x + box->size.x &&
            y < box->at.y + box->size.y;
}


pgb_t*
box_under(pgb_t *box, float x, float y)
{
    if (within(box, x, y)) {
        pgb_t     *tmp = NULL;
        pgb_t     *last = pgb_prop_get_bool(box, "can-focus")? box: NULL;
        float     adjusted_x = x - box->at.x;
        float     adjusted_y = y - box->at.y;
        if (!pgb_prop_get_bool(box, "opaque"))
            for (pgb_t *i = box->children; i; i = i->next)
                if ((tmp = box_under(i, adjusted_x, adjusted_y)))
                    last = tmp;
        return last;
    }
    return NULL;
}


PgPt
absolute_pos(const pgb_t *box)
{
    PgPt    p = pgpt(0, 0);
    for (const pgb_t *i = box; i; i = i->parent)
        p = pgpt(p.x + i->at.x, p.y + i->at.y);
    return p;
}


bool
pgb_event_handle(PgWindowEvent *e)
{
    if (!e)
        return false;

    pgb_t   *root = pgb_window_get_root(e->win);
    Pg      *canvas = pg_window_get_canvas(e->win);
    pgb_t   *hit;

    if (e->type == PG_EVENT_KEY_DOWN &&
        (!strcmp(e->key.key, "Ctrl+W") || !strcmp(e->key.key, "Escape")))
    {
        return false;
    }

    switch (e->type) {

    case PG_EVENT_MOUSE_MOVED:
        // Set hover.
        if ((hit = box_under(root, e->mouse.x, e->mouse.y)))
            pgb_set_hover(root, hit && hit != root? hit: NULL);

        if (pgb_get_capture(root))
            hit = pgb_get_capture(root);

        if (hit) {
            PgPt    abs = absolute_pos(hit);
            pgb_event(hit, "mouse-move", &(pgb_mouse_moved_t) {
                .x = e->mouse.x - abs.x,
                .y = e->mouse.y - abs.y,
            });
        }
        break;

    case PG_EVENT_MOUSE_DOWN:
        // Set focus.
        if ((hit = box_under(root, e->mouse.x, e->mouse.y)))
            pgb_set_focus(root, hit && hit != root? hit: NULL);

        if (pgb_get_capture(root))
            hit = pgb_get_capture(root);

        if (hit) {
            PgPt    abs = absolute_pos(hit);
            pgb_event(hit, "mouse-down", &(pgb_mouse_down_t) {
                .x = e->mouse.x - abs.x,
                .y = e->mouse.y - abs.y,
                .button = e->mouse.button,
            });
        }
        break;

    case PG_EVENT_MOUSE_UP:
        if (((hit = pgb_get_capture(root)) ||
             (hit = box_under(root, e->mouse.x, e->mouse.y))) &&
            hit != root)
        {
            PgPt    abs = absolute_pos(hit);
            pgb_event(hit, "mouse-up", &(pgb_mouse_up_t) {
                .x = e->mouse.x - abs.x,
                .y = e->mouse.y - abs.y,
                .button = e->mouse.button,
            });
        }
        break;

    case PG_EVENT_KEY_DOWN:
        if ((hit = pgb_get_capture(root)) || (hit = pgb_get_focus(root)))
            pgb_event(hit, "key-down", &(pgb_key_down_t) {
                .key = e->key.key,
            });
        break;

    case PG_EVENT_KEY_UP:
        if ((hit = pgb_get_capture(root)) || (hit = pgb_get_focus(root)))
            pgb_event(hit, "key-up", &(pgb_key_up_t) {
                .key = e->key.key,
            });
        break;

    case PG_EVENT_TEXT:
        if ((hit = pgb_get_capture(root)) || (hit = pgb_get_focus(root)))
            pgb_event(hit, "text", &(pgb_text_t) {
                .text = e->text.text,
            });
        break;

    case PG_EVENT_RESIZED:
        root->size = pgpt(e->resized.width, e->resized.height);
        pg_window_queue_update(e->win);
        break;

    case PG_EVENT_PAINT:
        pgb_pack(root);
        pgb_draw(root, canvas);
        pg_canvas_commit(canvas);
        pg_window_update(e->win);
        break;

    default:
        break;
    }

    return true;
}



pgb_t *disablable;
pgb_t *input;


bool
clicked(pgb_t *box, void *etc)
{
    (void) box;
    (void) etc;
    bool enabled = pgb_prop_get_bool(disablable, "enabled");
    pgb_prop_set_bool(disablable, "enabled", !enabled);
    puts("CLICKED!");
    return true;
}


int
main()
{
    setbuf(stdout, NULL);

    PgWindow *main_window = pg_window_open(1280, 720, "Box Demo");

pgb_t *menu;
    pgb_t *root_box = pgb_root(main_window);
    pgb_add(root_box,
        pgb_group(
            "major", "fill",
            "minor", "fill",
            "border-width", "2",
            "vertical", "true",
            "background", "cornsilk",
            "children",
            pgb_label(
                "minor", "center",
                "style", "title",
                "text", "Resident Evil: Director's Cut",
                NULL),
            pgb_label(
                "minor", "center",
                "style", "subtitle",
                "text", "Dual Shock Version",
                NULL),
            pgb_group(
                "minor", "center",
                // "background", "aliceblue",
                // "border-width", "2",
                "children",
                pgb_label("text", "Click OK", "minor", "center", NULL),
                disablable = pgb_button("text", "FINISH", NULL),
                NULL),
            pgb_button(
                "on-click", clicked,
                "children",
                pgb_label("text", "TEST LINK", NULL),
                pgb_label("text", "TEST LINK", NULL),
                NULL),
            pgb_group(
                "major", "fill",
                "minor", "fill",
                "background", "silver",
                "children",
                pgb_group(
                    "major", "fill",
                    "minor", "center",
                    "vertical", "true",
                    "children",
                    input = pgb_input(
                        "minor", "center",
                        "text", "Center",
                        NULL),
                    NULL),
                pgb_group(
                    "minor", "center",
                    "major", "fill",
                    "border-width", "2",
                    "children",
                    menu = pgb_listbox(NULL),
                    NULL),
                NULL),
            pgb_group(
                "ipad", "0",
                "minor", "end",
                "children",
                pgb_label("text", "Done?", "minor", "center", NULL),
                pgb_button("text", "Cancel", NULL),
                pgb_button("text", "OK", "on-click", clicked, NULL),
                NULL),
            NULL));

    pgb_set_focus(root_box, input);

    pgb_prop_set(input, "text", pg_font_prop_string(pgb_font(), PG_FONT_FAMILY));

    int n = 0;
    for (const PgFamily *i = pg_font_list(); i->name; i++)
        if (n++ < 10)
            pgb_add(menu, pgb_listbox_item("text", i->name, NULL));

    while (pgb_event_handle(pg_window_event_wait()));
}
