#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-box.h>


typedef struct Info {
    PgWindow    *win;
    PgBox       *hovered;
    PgBox       *focused;
    PgBox       *active;
} Info;



static Info*
get_info(PgBox *box)
{
    return pg_box_get_sys(box);
}

static void
paint(PgBox *box, Pg *g)
{
    Info *info = get_info(box);
    pg_canvas_state_reset(g);
    pg_canvas_set_clear(g, pg_paint_from_name("ui-bg"));
    pg_canvas_clear(g);
    pg_box_default_paint(box, g);
    pg_window_update(info->win);
}


static void
_free(PgBox *box)
{
    pg_window_free(pg_box_root_get_window(box));
}


static const PgBoxType type = {
    .type = "ROOT",
    .paint = paint,
    .free = _free,
};


PgBox*
pg_new_box_root(PgWindow *win, PgBoxFlags flags)
{
    if (!win)
        return NULL;

    return pg_box_set_sys(pg_box_new(&type, flags),
        pgnew(Info, .win = win));
}


PgWindow*
pg_box_root_get_window(PgBox *box)
{
    Info *info = get_info(box);
    return info? info->win: NULL;
}


PgBox*
pg_box_root_get_hovered(PgBox *root)
{
    Info *info = get_info(root);
    return info? info->hovered: NULL;
}


PgBox*
pg_box_root_get_focused(PgBox *root)
{
    Info *info = get_info(root);
    return info? info->focused: NULL;
}


PgBox*
pg_box_root_get_active(PgBox *root)
{
    Info *info = get_info(root);
    return info? info->active: NULL;
}


void
pg_box_update(PgBox *box)
{
    if (!box)
        return;

    PgBox *p = box;
    while (pg_box_get_type(p) != &type && pg_box_get_parent(p))
        p = pg_box_get_parent(p);

    if (p)
        pg_window_queue_update(pg_box_root_get_window(p));
}


void
pg_box_root_set_hovered(PgBox *root, PgBox *box)
{
    Info *info = get_info(root);
    if (!info)
        return;

    PgBox *old = info->hovered;
    info->hovered = box;
    if (old != box) {
        if (old && pg_box_get_type(old)->hovered)
            pg_box_get_type(old)->hovered(old, false);
        if (box && pg_box_get_type(box)->hovered)
            pg_box_get_type(box)->hovered(box, true);
    }
}


void
pg_box_root_set_focused(PgBox *root, PgBox *box)
{
    Info *info = get_info(root);
    if (!info)
        return;

    PgBox *old = info->focused;
    info->focused = box;
    if (old != box) {
        if (old && pg_box_get_type(old)->focused)
            pg_box_get_type(old)->focused(old, false);
        if (box && pg_box_get_type(box)->focused)
            pg_box_get_type(box)->focused(box, true);
    }
}


void
pg_box_root_set_active(PgBox *root, PgBox *box)
{
    Info *info = get_info(root);
    if (!info)
        return;

    PgBox *old = info->active;
    info->active = box;
    if (old != box) {
        if (old && pg_box_get_type(old)->active)
            pg_box_get_type(old)->active(old, false);
        if (box && pg_box_get_type(box)->active)
            pg_box_get_type(box)->active(box, true);
    }
}