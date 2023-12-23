#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3/pg.h>
#include <pg3/pg-internal-platform.h>
#include <pg3/pg-internal-window.h>


PgPt
pg_window_get_dpi(PgWindow *win)
{
    PgPt    dpi;

    if ((dpi = _pg_window_get_dpi_system(win)).x ||
        (dpi = _pg_window_get_dpi_platform(win)).x)
        return dpi;

    return pgpt(96.0f, 96.0f);
}


float
pg_window_get_dpi_x(PgWindow *win)
{
    return pg_window_get_dpi(win).x;
}


float
pg_window_get_dpi_y(PgWindow *win)
{
    return pg_window_get_dpi(win).y;
}


Pg*
pg_window_get_canvas(PgWindow *win)
{
    return win? win->g: NULL;
}


PgPt
pg_window_get_size(PgWindow *win)
{
    return win? pgpt(win->width, win->height): pgpt(0, 0);
}


float
pg_window_get_width(PgWindow *win)
{
    return win? win->width: 0.0f;
}


float
pg_window_get_height(PgWindow *win)
{
    return win? win->height: 0.0f;
}


PgWindow*
pg_window_open(unsigned width, unsigned height, const char *title)
{
    return _pg_window_open(width, height, title);
}


void
pg_window_close(PgWindow *win)
{
    if (win)
        _pg_window_close(win);
}


void
pg_window_free(PgWindow *win)
{
    if (!win)
        return;
    _pg_window_free(win);
    pg_canvas_free(win->g);
    free(win);
}


PgWindowEventType
pg_window_event_get_type(PgWindowEvent *e)
{
    return e? e->any.type: PG_EVENT_IGNORE;
}


PgWindow*
pg_window_event_get_window(PgWindowEvent *e)
{
    return e? e->any.win: NULL;
}


const char*
pg_window_event_get_key(PgWindowEvent *e)
{
    return e && (e->type == PG_EVENT_KEY_DOWN ||
                e->type == PG_EVENT_KEY_UP)
        ? e->key.key
        : NULL;
}


const char*
pg_window_event_get_text(PgWindowEvent *e)
{
    return e && e->type == PG_EVENT_TEXT? e->text.text: NULL;
}


PgPt
pg_window_event_get_mouse_pt(PgWindowEvent *e)
{
    return e && (e->type == PG_EVENT_MOUSE_DOWN ||
                e->type == PG_EVENT_MOUSE_UP ||
                e->type == PG_EVENT_MOUSE_MOVED ||
                e->type == PG_EVENT_MOUSE_WHEEL)
        ? pgpt(e->mouse.x, e->mouse.y)
        : pgpt(0.0f, 0.0f);
}


float
pg_window_event_get_mouse_x(PgWindowEvent *e)
{
    return e && (e->type == PG_EVENT_MOUSE_DOWN ||
                e->type == PG_EVENT_MOUSE_UP ||
                e->type == PG_EVENT_MOUSE_MOVED ||
                e->type == PG_EVENT_MOUSE_WHEEL)
        ? e->mouse.x
        : 0.0f;
}


float
pg_window_event_get_mouse_y(PgWindowEvent *e)
{
    return e && (e->type == PG_EVENT_MOUSE_DOWN ||
                e->type == PG_EVENT_MOUSE_UP ||
                e->type == PG_EVENT_MOUSE_MOVED ||
                e->type == PG_EVENT_MOUSE_WHEEL)
        ? e->mouse.y
        : 0.0f;
}


float
pg_window_event_get_mouse_wheel(PgWindowEvent *e)
{
    return e && (e->type == PG_EVENT_MOUSE_DOWN ||
                e->type == PG_EVENT_MOUSE_UP ||
                e->type == PG_EVENT_MOUSE_MOVED ||
                e->type == PG_EVENT_MOUSE_WHEEL)
        ? e->mouse.wheel
        : 0.0f;
}


const char*
pg_window_event_get_mouse_button(PgWindowEvent *e)
{
    return e && (e->type == PG_EVENT_MOUSE_DOWN ||
                e->type == PG_EVENT_MOUSE_UP ||
                e->type == PG_EVENT_MOUSE_MOVED ||
                e->type == PG_EVENT_MOUSE_WHEEL)
        ? e->mouse.button
        : NULL;
}


float
pg_window_event_get_resized_width(PgWindowEvent *e)
{
    return e && e->type == PG_EVENT_RESIZED? e->resized.width: 0.0f;
}


float
pg_window_event_get_resized_height(PgWindowEvent *e)
{
    return e && e->type == PG_EVENT_RESIZED? e->resized.height: 0.0f;
}

