#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-box.h>

struct map {
    PgWindow*   window;
    pgb_t*      box;
    pgb_t*      hover;
    pgb_t*      focus;
    pgb_t*      capture;
    struct map* next;
};

static struct map*      map;
static const pgb_type_t type;


static pgb_t*
associate(PgWindow *window, pgb_t *box)
{
    map = pgnew(struct map, .window=window, .box=box, .next=map);
    return box;
}


static struct map*
lookup_box(const pgb_t *box)
{
    for (struct map *i = map; i; i = i->next)
        if (i->box == box) return i;
    return NULL;
}


static struct map*
lookup_window(const PgWindow *window)
{
    for (struct map *i = map; i; i = i->next)
        if (i->window == window) return i;
    return NULL;
}


pgb_t*
pgb_root(PgWindow *window)
{
    if (!window)
        return NULL;
    return associate(window, pgb_box(&type, NULL));
}


pgb_t*
pgb_get_root(const pgb_t *box)
{
    if (!box)
        return NULL;

    pgb_t *i = (pgb_t*) box;
    while (i->parent)
        i = i->parent;
    return i;
}


pgb_t*
pgb_set_focus(pgb_t *root, pgb_t *box)
{
    struct map *map = lookup_box(pgb_get_root(root));
    if (map) {
        pgb_t *old = map->focus;
        map->focus = box;
        if (box != old)
            pgb_event(old, "focus-off", NULL),
            pgb_event(box, "focus", NULL),
            pgb_update(root);
        return old;
    }
    return NULL;
}


pgb_t*
pgb_set_hover(pgb_t *root, pgb_t *box)
{
    struct map *map = lookup_box(pgb_get_root(root));
    if (map) {
        pgb_t *old = map->hover;
        map->hover = box;
        if (box != old)
            pgb_event(old, "hover-off", NULL),
            pgb_event(box, "hover", NULL),
            pgb_update(root);
        return old;
    }
    return NULL;
}


pgb_t*
pgb_set_capture(pgb_t *root, pgb_t *box)
{
    struct map *map = lookup_box(pgb_get_root(root));
    if (map) {
        pgb_t *old = map->capture;
        map->capture = box;
        if (box != old)
            pgb_event(old, "capture-off", NULL),
            pgb_event(box, "capture", NULL);
        return old;
    }
    return NULL;
}


pgb_t*
pgb_get_focus(const pgb_t *root)
{
    struct map *map = lookup_box(pgb_get_root(root));
    return map? map->focus: NULL;
}


pgb_t*
pgb_get_hover(const pgb_t *root)
{
    struct map *map = lookup_box(pgb_get_root(root));
    return map? map->hover: NULL;
}


pgb_t*
pgb_get_capture(const pgb_t *root)
{
    struct map *map = lookup_box(pgb_get_root(root));
    return map? map->capture: NULL;
}


bool
pgb_is_focus(const pgb_t *box)
{
    return pgb_get_focus(box) == box;
}


bool
pgb_is_hover(const pgb_t *box)
{
    return pgb_get_hover(box) == box;
}


bool
pgb_is_capture(const pgb_t *box)
{
    return pgb_get_capture(box) == box;
}


pgb_t*
pgb_window_get_root(PgWindow *window)
{
    struct map *map = lookup_window(window);
    return map? map->box: NULL;
}


PgWindow*
pgb_root_get_window(const pgb_t *box)
{
    struct map *map = lookup_box(box);
    return map? map->window: NULL;
}


void
pgb_update(pgb_t *box)
{
    pg_window_queue_update(pgb_root_get_window(pgb_get_root(box)));
}


static void
draw(pgb_t *box, Pg *g)
{
    pg_canvas_set_clear(g, pg_paint_from_name("ui-bg"));
    pg_canvas_clear(g);
    pgb_default_draw(box, g);
}


static const pgb_type_t
type = {
    "root",
    .draw = draw,
};

