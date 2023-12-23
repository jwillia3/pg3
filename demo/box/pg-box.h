typedef struct pgb_t            pgb_t;
typedef struct pgb_type_t       pgb_type_t;
typedef struct pgb_handler_t    pgb_handler_t;
typedef struct pgb_prop_t       pgb_prop_t;

typedef struct pgb_mouse_moved_t    pgb_mouse_moved_t;
typedef struct pgb_mouse_down_t     pgb_mouse_down_t;
typedef struct pgb_mouse_up_t       pgb_mouse_up_t;
typedef struct pgb_key_down_t       pgb_key_down_t;
typedef struct pgb_key_up_t         pgb_key_up_t;
typedef struct pgb_text_t           pgb_text_t;

struct pgb_t {
    const pgb_type_t*   type;
    PgPt                at;
    PgPt                size;
    pgb_t*              parent;
    pgb_t*              children;
    pgb_t*              next;
    pgb_handler_t*      handlers;
    pgb_prop_t*         props;
};

struct pgb_type_t {
    const char* name;
    PgPt        (*measure)(const pgb_t *box);
    void        (*pack)(pgb_t *box);
    void        (*draw)(pgb_t *box, Pg *canvas);
    void        (*free)(pgb_t *box);
};

struct pgb_handler_t {
    const char*     event;
    bool            (*handle)(pgb_t *box, void *etc);
    pgb_handler_t*   next;
};

struct pgb_mouse_moved_t {
    float           x;
    float           y;
};

struct pgb_mouse_down_t {
    float           x;
    float           y;
    const char*     button;
};

struct pgb_mouse_up_t {
    float           x;
    float           y;
    const char*     button;
};

struct pgb_key_down_t {
    const char*     key;
};

struct pgb_key_up_t {
    const char*     key;
};

struct pgb_text_t {
    const char*     text;
};


/*
    Constructors and Destructors.
 */
void        pgb_free(pgb_t *box);
pgb_t*      pgb_box(const pgb_type_t *type, ...);
pgb_t*      pgb_root(PgWindow *window);
pgb_t*      pgb_group(const char *arg, ...);
pgb_t*      pgb_label(const char *arg, ...);
pgb_t*      pgb_button(const char *arg, ...);
pgb_t*      pgb_input(const char *arg, ...);
pgb_t*      pgb_listbox(const char *arg, ...);
pgb_t*      pgb_listbox_item(const char *arg, ...);


/*
    Properties.
 */
pgb_t*      pgb_props_set(pgb_t *box, ...);
pgb_t*      pgb_props_set_var(pgb_t *box, const char *name, va_list ap);
pgb_t*      pgb_prop_set(pgb_t *box, const char *name, const char *value);
pgb_t*      pgb_prop_set_float(pgb_t *box, const char *name, float value);
pgb_t*      pgb_prop_set_bool(pgb_t *box, const char *name, bool value);
const char* pgb_prop_get(const pgb_t *box, const char *name);
float       pgb_prop_get_float(const pgb_t *box, const char *name);
bool        pgb_prop_get_bool(const pgb_t *box, const char *name);


/*
    Relationships and Events.
 */
pgb_t*      pgb_add(pgb_t *parent, pgb_t *child);
pgb_t*      pgb_adds(pgb_t *parent, ...);
void        pgb_remove(pgb_t *child);
pgb_t*      pgb_handle(pgb_t *box, const char *event, bool (*handle)(pgb_t *box, void *etc));
pgb_t*      pgb_handles(pgb_t *box, ...);
bool        pgb_event(pgb_t *box, const char *event, void *etc);


/*
    Etc.
 */
const char* pgb_get_type(const pgb_t *box);
pgb_t*      pgb_hit(pgb_t *box, float x, float y);
PgPt        pgb_absolute_pos(const pgb_t *box);
pgb_t*      pgb_get_root(const pgb_t *box);
pgb_t*      pgb_set_focus(pgb_t *root, pgb_t *box);
pgb_t*      pgb_set_hover(pgb_t *root, pgb_t *box);
pgb_t*      pgb_set_capture(pgb_t *root, pgb_t *box);
pgb_t*      pgb_get_focus(const pgb_t *root);
pgb_t*      pgb_get_hover(const pgb_t *root);
pgb_t*      pgb_get_capture(const pgb_t *root);
bool        pgb_is_focus(const pgb_t *box);
bool        pgb_is_hover(const pgb_t *box);
bool        pgb_is_capture(const pgb_t *box);


/*
    Functions for implementation.
 */
PgFont*     pgb_font(void);
PgFont*     pgb_monospace_font(void);
PgFont*     pgb_subtitle_font(void);
PgFont*     pgb_title_font(void);

PgPt        pgb_measure(const pgb_t *box);
void        pgb_pack(pgb_t *box);
void        pgb_draw(pgb_t *box, Pg *canvas);
PgPt        pgb_restrict(const pgb_t *box, PgPt size);
void        pgb_update(pgb_t *box);

PgPt        pgb_default_measure(const pgb_t *box);
void        pgb_default_pack(pgb_t *box);
void        pgb_default_draw(pgb_t *box, Pg *g);

PgWindow*   pgb_root_get_window(const pgb_t *box);
pgb_t*      pgb_window_get_root(PgWindow *win);
