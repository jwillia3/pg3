struct PgWindow {
    Pg              *g;
    unsigned        width;
    unsigned        height;
    const char      *title;
    PgWindowEvent   e;
    PgWindowEvent   q;
    bool            queued;
    double          last_motion;
};


/*
    Implement this function.
 */
PgWindow*   _pg_window_open(unsigned width, unsigned height, const char *title);
void        _pg_window_close(PgWindow *win);
void        _pg_window_free(PgWindow *win);
PgPt        _pg_window_get_dpi_system(PgWindow *win);
void        _pg_window_set_size(PgWindow *win, unsigned width, unsigned height);
void        _pg_window_set_title(PgWindow *win, const char *title);
