#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-platform.h>
#include <pg-internal-window.h>


PgPt
pg_window_get_dpi(PgWindow *win)
{
    PgPt    dpi;

    if ((dpi = _pg_window_get_dpi_system(win)).x ||
        (dpi = _pg_window_get_dpi_platform(win)).x)
        return dpi;

    return pgpt(96.0f, 96.0f);
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