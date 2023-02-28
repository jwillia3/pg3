#include <pg3.h>


void*
pg_get_user(Pg *g)
{
    return g? g->user: 0;
}


void
pg_set_user(Pg *g, void *ptr)
{
    if (g)
        g->user = ptr;
}


void*
pg_get_sys(Pg *g)
{
    return g? g->sys: 0;
}


void
pg_set_sys(Pg *g, void *ptr)
{
    if (g)
        g->sys = ptr;
}


Pgb*
pg_get_box(Pg *g)
{
    if (!g) return 0;
    return g->box;
}


void
pg_set_box(Pg *g, Pgb *box)
{
    if (!g) return;
    g->box = box;
}

#include <signal.h>
bool
pg_enqueue_redraw(Pg *g)
{
    if (!g) raise(SIGINT);
    return pg_enqueue((PgEvent) { PG_REDRAW_EVENT, g, {0} });
}


bool
pg_enqueue_resized(Pg *g, int width, int height)
{
    return pg_enqueue((PgEvent) {
        PG_RESIZE_EVENT,
        g,
        .resized= {
            .sx = width,
            .sy = height
        }
    });
}


bool
pg_enqueue_closed(Pg *g)
{
    return pg_enqueue((PgEvent) { PG_CLOSE_EVENT, g, {0} });
}


bool
pg_enqueue_key_down(Pg *g, int key, PgMods mods)
{
    return pg_enqueue((PgEvent) {
        PG_KEY_DOWN_EVENT,
        g,
        .key = {
            .key = key,
            .mods = mods,
        }
    });
}


bool
pg_enqueue_key_up(Pg *g, int key, PgMods mods)
{
    return pg_enqueue((PgEvent) {
        PG_KEY_UP_EVENT,
        g,
        .key = {
            .key = key,
            .mods = mods,
        }
    });
}


bool
pg_enqueue_char(Pg *g, unsigned codepoint)
{
    return pg_enqueue((PgEvent) {
        PG_CHAR_EVENT,
        g,
        .codepoint = codepoint,
    });
}


bool
pg_enqueue_mouse_down(Pg *g, int x, int y, int button, PgMods mods)
{
    return pg_enqueue((PgEvent) {
        PG_MOUSE_DOWN_EVENT,
        g,
        .mouse = {
            .x = x,
            .y = y,
            .button = button,
            .mods = mods,
        }
    });
}


bool
pg_enqueue_mouse_up(Pg *g, int x, int y, int button, PgMods mods)
{
    return pg_enqueue((PgEvent) {
        PG_MOUSE_UP_EVENT,
        g,
        .mouse = {
            .x = x,
            .y = y,
            .button = button,
            .mods = mods,
        }
    });
}


bool
pg_enqueue_files_dropped(Pg *g, int npaths, const char **paths)
{
    return pg_enqueue((PgEvent) {
        PG_FILES_DROPPED_EVENT,
        g,
        .dropped = {
            .npaths = npaths,
            .paths = (const char**) paths,
        }
    });
}


bool
pg_enqueue_mouse_move(Pg *g, int x, int y, PgMods mods)
{
    return pg_enqueue((PgEvent) {
        PG_MOUSE_MOVE_EVENT,
        g,
        .mouse = {
            .x = x,
            .y = y,
            .mods = mods,
        }
    });
}


bool
pg_enqueue_scrolled(Pg *g, float x, float y, float dx, float dy, PgMods mods)
{
    return pg_enqueue((PgEvent) {
        PG_MOUSE_SCROLL_EVENT,
        g,
        .mouse = {
            .x = x,
            .y = y,
            .dx = dx,
            .dy = dy,
            .mods = mods,
        }
    });
}
