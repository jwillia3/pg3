typedef struct PgBox        PgBox;
typedef struct PgBoxType    PgBoxType;

typedef enum PgBoxFlags {
    PG_BOX_FLAG_MAJOR_FILL =    0x08,

    PG_BOX_FLAG_MINOR_MASK =    0x03,
    PG_BOX_FLAG_MINOR_START =   0x00,
    PG_BOX_FLAG_MINOR_CENTER =  0x01,
    PG_BOX_FLAG_MINOR_END =     0x02,
    PG_BOX_FLAG_MINOR_FILL =    0x03,

    PG_BOX_FLAG_NO_PAD =        0x10,

    PG_BOX_FLAG_VERT =          0x20,

    PG_BOX_FLAG_TARGET =        0x40,

    PG_BOX_FLAG_FILL =          PG_BOX_FLAG_MAJOR_FILL | PG_BOX_FLAG_MINOR_FILL,
} PgBoxFlags;


bool        pg_event_handle(PgBox *root, PgWindowEvent *e);
PgBox*      pg_box_get_from_loc(PgBox *root, float x, float y);
void        pg_box_update(PgBox *box);
void        pg_box_free(PgBox *box, bool free_children);


/*
    Boxes:

    Root        the root box for a window
    Canvas      a box for custom painting
    Group       group of boxes
    Border      bordered group of boxes
    Text

*/
PgBox*      pg_new_box_root(PgWindow *win, PgBoxFlags flags);
PgBox*      pg_new_canvas_box(void (*paint)(PgBox *box, Pg *g), PgBoxFlags flags);
PgBox*      pg_box_new_group(PgBoxFlags flags);
PgBox*      pg_box_new_vgroup(PgBoxFlags flags);
PgBox*      pg_box_grouped(PgBoxFlags flags, PgBox **children);
PgBox*      pg_box_vgrouped(PgBoxFlags flags, PgBox **children);
PgBox*      pg_box_new_border(PgBoxFlags flags);
PgBox*      pg_box_new_vborder(PgBoxFlags flags);
PgBox*      pg_box_bordered(PgBoxFlags flags, PgBox **children);
PgBox*      pg_box_vbordered(PgBoxFlags flags, PgBox **children);
PgBox*      pg_box_pad(void);
PgBox*      pg_box_fill(void);
PgBox*      pg_box_text(PgBoxFlags flags, const char *text);
PgBox*      pg_box_title(PgBoxFlags flags, const char *text);
PgBox*      pg_box_subtitle(PgBoxFlags flags, const char *text);
PgBox*      pg_box_heading(PgBoxFlags flags, const char *text);
PgBox*      pg_box_button(PgBoxFlags flags);
PgBox*      pg_box_button_text(PgBoxFlags flags, const char *text);
PgBox*      pg_box_buttoned(PgBoxFlags flags, PgBox **children);


/*

*/
PgWindow*   pg_box_root_get_window(PgBox *box);
PgBox*      pg_box_root_get_hovered(PgBox *root);
PgBox*      pg_box_root_get_focused(PgBox *root);
PgBox*      pg_box_root_get_active(PgBox *root);

void        pg_box_root_set_hovered(PgBox *root, PgBox *box);
void        pg_box_root_set_focused(PgBox *root, PgBox *box);
void        pg_box_root_set_active(PgBox *root, PgBox *box);


// Text
const char* pg_box_text_get_text(PgBox *box);
PgFont*     pg_box_text_get_font(PgBox *box);
PgPaint*    pg_box_text_get_fg(PgBox *box);
PgBox*      pg_box_text_set_text(PgBox *box, const char *text);
PgBox*      pg_box_text_set_font(PgBox *box, PgFont *font);
PgBox*      pg_box_text_set_fg(PgBox *box, PgPaint *paint);




PgPt        pg_box_measure(const PgBox *box);
void        pg_box_pack(PgBox *box);
void        pg_box_paint(PgBox *box, Pg *g);


/*
    Manipulation
*/
void        pg_box_add(PgBox *parent, PgBox *child);
PgBox*      pg_box_added(PgBox *parent, PgBox **children);
void        pg_box_remove(PgBox *child);


/*
    Properties
*/
PgPt        pg_box_get_min_size(const PgBox *box);
PgPt        pg_box_get_size(const PgBox *box);
PgPt        pg_box_get_loc(const PgBox *box);
PgBoxFlags  pg_box_get_flags(const PgBox *box);
PgBox*      pg_box_get_parent(const PgBox *box);
PgBox*      pg_box_get_child(const PgBox *box);
PgBox*      pg_box_get_next(const PgBox *box);
void*       pg_box_get_sys(const PgBox *box);
void*       pg_box_get_user(const PgBox *box);
const PgBoxType* pg_box_get_type(const PgBox *box);

PgBox*      pg_box_set_min_size(PgBox *box, float x, float y);
PgBox*      pg_box_set_size(PgBox *box, float x, float y);
PgBox*      pg_box_set_loc(PgBox *box, float x, float y);
PgBox*      pg_box_set_flags(PgBox *box, PgBoxFlags flags);
PgBox*      pg_box_set_sys(PgBox *box, void *sys);
PgBox*      pg_box_set_user(PgBox *box, void *user);

/*
    Implementation
*/
PgBox*      pg_box_new(const PgBoxType *type, PgBoxFlags flags);
PgPt        pg_box_default_measure(const PgBox *box);
void        pg_box_default_pack(PgBox *box);
void        pg_box_default_paint(PgBox *box, Pg *g);
PgFont*     pg_box_font_text(void);
PgFont*     pg_box_font_title(void);
PgFont*     pg_box_font_subtitle(void);
PgFont*     pg_box_font_heading(void);