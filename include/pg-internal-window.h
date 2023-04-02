struct PgWindow {
    Pg              *g;
    unsigned        width;
    unsigned        height;
    PgWindowEvent   e;
    PgWindowEvent   q;
    bool            queued;
};


/*
    Implement this function.
 */
PgWindow*   _pg_window_open(unsigned width, unsigned height, const char *title);
void        _pg_window_close(PgWindow *win);
void        _pg_window_free(PgWindow *win);
PgPt        _pg_window_get_dpi_system(PgWindow *win);
