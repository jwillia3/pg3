typedef struct PgWindow     PgWindow;

typedef union PgWindowEvent         PgWindowEvent;
typedef struct PgWindowEventAny     PgWindowEventAny;
typedef struct PgWindowEventResized PgWindowEventResized;
typedef struct PgWindowEventShown   PgWindowEventShown;
typedef struct PgWindowEventKey     PgWindowEventKey;
typedef struct PgWindowEventText    PgWindowEventText;
typedef struct PgWindowEventMouse   PgWindowEventMouse;
typedef enum {
    PG_WINDOW_EVENT_IGNORE,
    PG_WINDOW_EVENT_PAINT,
    PG_WINDOW_EVENT_RESIZED,
    PG_WINDOW_EVENT_CLOSED,
    PG_WINDOW_EVENT_KEY_DOWN,
    PG_WINDOW_EVENT_KEY_UP,
    PG_WINDOW_EVENT_TEXT,
    PG_WINDOW_EVENT_MOUSE_DOWN,
    PG_WINDOW_EVENT_MOUSE_UP,
    PG_WINDOW_EVENT_MOUSE_MOVED,
    PG_WINDOW_EVENT_MOUSE_WHEEL,
    PG_WINDOW_EVENT_USER,
} PgWindowEventType;

struct PgWindowEventAny {
    PgWindow            *win;
    PgWindowEventType   type;
};

struct PgWindowEventResized {
    PgWindow            *win;
    PgWindowEventType   type;
    unsigned            width;
    unsigned            height;
};

struct PgWindowEventKey {
    PgWindow            *win;
    PgWindowEventType   type;
    const char          *key;
};

struct PgWindowEventText {
    PgWindow            *win;
    PgWindowEventType   type;
    const char          *text;
    char                tmp[8];
};

struct PgWindowEventMouse {
    PgWindow            *win;
    PgWindowEventType   type;
    float               x;
    float               y;
    float               wheel;
    const char          *button;
};

union PgWindowEvent {
    struct {
        PgWindow            *win;
        PgWindowEventType   type;
    };
    PgWindowEventAny        any;
    PgWindowEventResized    resized;
    PgWindowEventKey        key;
    PgWindowEventText       text;
    PgWindowEventMouse      mouse;
};



PgWindow*   pg_window_open(unsigned width, unsigned height, const char *title);
void        pg_window_free(PgWindow *win);
void        pg_window_close(PgWindow *win);

PgPt        pg_window_get_dpi(PgWindow *win);
float       pg_window_get_dpi_x(PgWindow *win);
float       pg_window_get_dpi_y(PgWindow *win);
Pg*         pg_window_get_canvas(PgWindow *win);
PgPt        pg_window_get_size(PgWindow *win);
float       pg_window_get_width(PgWindow *win);
float       pg_window_get_height(PgWindow *win);
void        pg_window_update(PgWindow *win);
void        pg_window_queue_update(PgWindow *win);
void        pg_window_queue_dummy(PgWindow *win);

PgWindowEvent*  pg_window_event_wait(void);

PgWindowEventType pg_window_event_get_type(PgWindowEvent *e);
PgWindow*   pg_window_event_get_window(PgWindowEvent *e);
const char* pg_window_event_get_key(PgWindowEvent *e);
const char* pg_window_event_get_text(PgWindowEvent *e);
PgPt        pg_window_event_get_mouse_pt(PgWindowEvent *e);
float       pg_window_event_get_mouse_x(PgWindowEvent *e);
float       pg_window_event_get_mouse_y(PgWindowEvent *e);
float       pg_window_event_get_mouse_wheel(PgWindowEvent *e);
const char* pg_window_event_get_mouse_button(PgWindowEvent *e);
float       pg_window_event_get_resized_width(PgWindowEvent *e);
float       pg_window_event_get_resized_height(PgWindowEvent *e);
