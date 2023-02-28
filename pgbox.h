typedef struct PgbFn PgbFn;

typedef enum PgbFlags {
    PGB_CROSS =             0x0003,
    PGB_START_CROSS =       0x0000,
    PGB_FILL_CROSS =        0x0001,
    PGB_CENTER_CROSS =      0x0002,
    PGB_END_CROSS =         0x0003,

    PGB_FILL_AXIS =         0x0004,

    PGB_FIXED_POS =         0x0008,

    PGB_PACK =              0x0030,
    PGB_PACK_VERT =         0x0000,
    PGB_PACK_HORIZ =        0x0010,
    PGB_PACK_NONE =         0x0020,

    PGB_NO_IPAD =           0x0040,

    PGB_WRAP =              0x0080,

    PGB_OPAD_AXIS_START =   0x0100,
    PGB_OPAD_CROSS_START =  0x0200,
    PGB_OPAD_AXIS_END =     0x0400,
    PGB_OPAD_CROSS_END =    0x0800,

    PGB_FILL =              PGB_FILL_AXIS | PGB_FILL_CROSS,
    PGB_OPAD_AXIS =         PGB_OPAD_AXIS_START | PGB_OPAD_AXIS_END,
    PGB_OPAD_CROSS =        PGB_OPAD_CROSS_START | PGB_OPAD_CROSS_END,
    PGB_OPAD =              PGB_OPAD_AXIS | PGB_OPAD_CROSS,
} PgbFlags;

struct PgbFn {
    /*
        WARNING:

        DO NOT ADD TO OR CHANGE THE ORDER OF THESE
        WITHOUT UPDATING pgb_merge().

     */
    void (*draw)(Pgb *box, Pg *g);
    void (*free)(Pgb *box);
    void (*autosize)(Pgb *box);
    void (*pack)(Pgb *box, float availx, float availy);
    void (*click)(Pgb *box, float x, float y, int button, PgMods mods);
    void (*mouse_down)(Pgb *box, float x, float y, int button, PgMods mods);
    void (*mouse_up)(Pgb *box, float x, float y, int button, PgMods mods);
    void (*mouse_move)(Pgb *box, float x, float y, int button, PgMods mods);
    void (*key_down)(Pgb *box, PgKey key, PgMods mods);
    void (*key_up)(Pgb *box, PgKey key, PgMods mods);
    void (*character)(Pgb *box, unsigned codepoint);
    void (*hover)(Pgb *box, bool over);
    void (*focus)(Pgb *box, bool on);
    void (*drag)(Pgb *box, bool dragging);
    void (*drop)(Pgb *box);
    Pgb *(*hit)(Pgb *box, float x, float y, PgPt *adjusted);
    void (*dirty)(Pgb *box);
};

struct Pgb {
    const PgbFn *v;
    float       x, y;       // Location
    float       sx, sy;     // Total calculated size
    float       cx, cy;     // Content size
    Pgb         *next;
    Pgb         *child;
    Pgb         *parent;
    void        *user;
    void        *sys;
    PgbFlags    flags;
};

typedef enum PgbColor {
    PGB_NO_COLOR,
    PGB_LIGHT_COLOR,
    PGB_MID_COLOR,
    PGB_DARK_COLOR,
    PGB_ACCENT_COLOR,
} PgbColor;

typedef enum PgbFont {
    PGB_DEFAULT_FONT,
    PGB_HEADER_FONT,
} PgbFont;


PgPaint *pgb_color(PgbColor colour);
PgFont  *pgb_font(PgbFont font);


/*

    Creating Boxes

*/
Pgb *pgb_root(Pg *g);
Pgb *pgb_box(float cx, float cy, PgbFlags flags, const PgbFn *v);
Pgb *pgb_space(float cx, float cy, PgbFlags flags);
Pgb *pgb_pad(void);
Pgb *pgb_fill(void);
Pgb *pgb_group(Pgb *content, PgbFlags flags);
Pgb *pgb_grouped(PgbFlags flags, Pgb **content);
Pgb *pgb_hgroup(Pgb *content, PgbFlags flags);
Pgb *pgb_hgrouped(PgbFlags flags, Pgb **content);
Pgb *pgb_border(Pgb *content, PgbFlags flags);
Pgb *pgb_bordered(PgbFlags flags, Pgb **content);
Pgb *pgb_label(const char *text, PgbFlags flags);
Pgb *pgb_header(const char *text, PgbFlags flags);
Pgb *pgb_textbox(const char *text,
        void (*enter)(Pgb *box, const char *text),
        PgbFlags flags);
Pgb *pgb_menu_item(const char *label, void *value);
Pgb *pgb_menu(PgbFlags flags, void (*selected)(Pgb *box, void *value), Pgb **items);
Pgb *pgb_canvas(float x,
                float y,
                void (*draw)(Pgb *box, Pg *g),
                PgbFlags flags);
Pgb *pgb_scrollbar(bool horiz,
              float value,
              void (*scrolled)(Pgb *box, float value),
              PgbFlags flags);
Pgb *pgb_scrollable(int cx, int cy, Pgb *content, PgbFlags flags);
Pgb *pgb_color_picker(
        void (*done)(Pgb *box, PgColorSpace cspace, PgColor color),
        PgbFlags flags);


PgbFn pgb_merge(PgbFn to, PgbFn from);
PgbFn pgb_defaults(void);
void pgb_default_autosize(Pgb *box);
void pgb_default_pack(Pgb *box, float ax, float ay);
void pgb_default_draw(Pgb *box, Pg *g);




PgbColor pgb_get_border_color(Pgb *box);
void pgb_set_border_color(Pgb *box, PgbColor color);



Pgb *pgb_button(const char *text, void (*clicked)(Pgb *box), PgbFlags flags);
bool pgb_is_button_enabled(Pgb *box);
Pgb *pgb_enable_button(Pgb *box, bool enabled);
Pgb *pgb_set_button_text(Pgb *box, const char *text);
const char *pgb_get_button_text(Pgb *box);
void pgb_set_on_button_hover(Pgb *box, void (*hover)(Pgb *box, bool over));
void pgb_set_on_button_focus(Pgb *box, void (*focus)(Pgb *box, bool on));



Pgb *pgb_set_label_text(Pgb *box, const char *text);
Pgb *pgb_set_label_font(Pgb *box, PgbFont font);
Pgb *pgb_set_label_color(Pgb *box, PgbColor color);

const char* pgb_get_label_text(Pgb *box);
PgbFont pgb_get_label_font(Pgb *box);
PgbColor pgb_get_label_color(Pgb *box);



char *pgb_get_textbox_text(Pgb *box);
Pgb *pgb_set_textbox_text(Pgb *box, const char *text);
Pgb *pgb_on_update_textbox(Pgb *box,
        void (*updated)(Pgb *box, const char *text));
bool pgb_is_textbox_enabled(Pgb *box);
Pgb *pgb_enable_textbox(Pgb *box, bool enabled);


Pgb *pgb_checkbox(const char *label,
                  bool checked,
                  void (*clicked)(Pgb *box),
                  PgbFlags flags);
bool pgb_is_checkbox_checked(Pgb *box);
void pgb_check_checkbox(Pgb *box, bool checked);
bool pgb_is_checkbox_enabled(Pgb *box);
Pgb *pgb_enable_checkbox(Pgb *box, bool enabled);


void pgb_draw(Pg *g, Pgb *box);
void pgb_autosize(Pgb *box);
void pgb_pack(Pgb *box, float availx, float availy);

void *pgb_get_user(Pgb *box);
void *pgb_get_sys(Pgb *box);
Pgb *pgb_set_user(Pgb *box, void *ptr);
Pgb *pgb_set_sys(Pgb *box, void *ptr);

void pgb_dirty(Pgb *box);
void pgb_free(Pgb *box);
Pgb *pgb_add(Pgb *parent, Pgb *child);
Pgb *pgb_add_list(Pgb *parent, Pgb **children);
Pgb *pgb_addv(Pgb *parent, va_list ap);
Pgb *pgb_add_all(Pgb *parent, ...);
void pgb_remove(Pgb *child);

bool pgb_point_in(Pgb *box, float x, float y);
Pgb *pgb_hit(Pgb *box, float x, float y, PgPt *adjusted);
PgPt pgb_abs_pos(Pgb *child);


/*
    Standard UI Events and Mechanics.
    Hovering, Focus, and Dragging.
*/

void pgb_std_events(PgEvent e);

void pgb_set_hovered(Pgb *box);
Pgb *pgb_get_hovered(void);
bool pgb_is_hovered(Pgb *box);

void pgb_set_focused(Pgb *box);
Pgb *pgb_get_focused(void);
bool pgb_is_focused(Pgb *box);

void pgb_set_dragged(Pgb *box);
Pgb *pgb_get_dragged(void);
bool pgb_is_dragged(Pgb *box);
void pgb_drop(void);
void pgb_set_drag_offset(float x, float y);
PgPt pgb_get_drag_offset(void);
void pgb_bring_to_front(Pgb *box);
void pgb_clear_status(Pgb *box);


/*
    Handling Events.
*/
void pgb_on_draw(Pgb *box, Pg *g);
void pgb_on_free(Pgb *box);
void pgb_on_autosize(Pgb *box);
void pgb_on_pack(Pgb *box, float availx, float availy);
void pgb_on_click(Pgb *box, float x, float y, int button, PgMods mods);
void pgb_on_mouse_down(Pgb *box, float x, float y, int button, PgMods mods);
void pgb_on_mouse_up(Pgb *box, float x, float y, int button, PgMods mods);
void pgb_on_mouse_move(Pgb *box, float x, float y, int button, PgMods mods);
void pgb_on_key_down(Pgb *box, PgKey key, PgMods mods);
void pgb_on_key_up(Pgb *box, PgKey key, PgMods mods);
void pgb_on_character(Pgb *box, unsigned codepoint);
void pgb_on_hover(Pgb *box, bool over);
void pgb_on_focus(Pgb *box, bool on);
void pgb_on_drag(Pgb *box, bool dragging);
void pgb_on_drop(Pgb *box);
void pgb_on_dirty(Pgb *box);
Pgb *pgb_on_hit(Pgb *box, float x, float y, PgPt *adjusted);
