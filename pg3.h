#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct Pg           Pg;             // Canvas
typedef struct PgState      PgState;        // State of canvas
typedef struct PgSubcanvas  PgSubcanvas;    // Limited section of another canvas
typedef struct PgGroup      PgGroup;        // Control group
typedef struct PgColor      PgColor;        // Colour
typedef struct PgPt         PgPt;           // Point
typedef struct PgTM         PgTM;           // Transform Matrix (2D)
typedef struct PgPath       PgPath;         // Path
typedef struct PgPart       PgPart;         // Part of path
typedef struct PgPaint      PgPaint;        // Paint
typedef struct PgFont       PgFont;         // Font
typedef struct PgFace       PgFace;         // Description of font face
typedef struct PgFamily     PgFamily;       // Description of font family

typedef struct PgCanvasImpl PgCanvasImpl;
typedef struct PgFontImpl   PgFontImpl;


enum PgPartType {
    PG_PART_MOVE,
    PG_PART_LINE,
    PG_PART_CURVE3,
    PG_PART_CURVE4,
    PG_PART_CLOSE,
};

enum PgPaintType {
    PG_SOLID_PAINT,
    PG_LINEAR_PAINT,
};

enum PgColorSpace {
    PG_SRGB,
    PG_LCHAB,
    PG_LAB,
    PG_XYZ,
};

struct PgColor {
    float x;
    float y;
    float z;
    float a;
};

enum PgLineCap {
    PG_BUTT_CAP,
    PG_SQUARE_CAP,
};

enum PgFillRule {
    PG_NONZERO_RULE,
    PG_EVEN_ODD_RULE,
};

enum PgTextPos {
    PG_TEXT_POS_TOP,
    PG_TEXT_POS_BOTTOM,
    PG_TEXT_POS_BASELINE,
    PG_TEXT_POS_CENTER,
};

enum PgFontProp {
    PG_FONT_FORMAT,
    PG_FONT_INDEX,
    PG_FONT_NFONTS,
    PG_FONT_FAMILY,
    PG_FONT_STYLE,
    PG_FONT_FULL_NAME,
    PG_FONT_IS_FIXED,
    PG_FONT_IS_ITALIC,
    PG_FONT_IS_SANS_SERIF,
    PG_FONT_IS_SERIF,
    PG_FONT_WEIGHT,
    PG_FONT_WIDTH_CLASS,
    PG_FONT_STYLE_CLASS,
    PG_FONT_STYLE_SUBCLASS,
    PG_FONT_ANGLE,
    PG_FONT_PANOSE_1,
    PG_FONT_PANOSE_2,
    PG_FONT_PANOSE_3,
    PG_FONT_PANOSE_4,
    PG_FONT_PANOSE_5,
    PG_FONT_PANOSE_6,
    PG_FONT_PANOSE_7,
    PG_FONT_PANOSE_8,
    PG_FONT_PANOSE_9,
    PG_FONT_PANOSE_10,
    PG_FONT_NGLYPHS,
    PG_FONT_UNITS,
    PG_FONT_EM,
    PG_FONT_AVG_WIDTH,
    PG_FONT_ASCENDER,
    PG_FONT_DESCENDER,
    PG_FONT_LINEGAP,
    PG_FONT_XHEIGHT,
    PG_FONT_CAPHEIGHT,
    PG_FONT_UNDERLINE,
    PG_FONT_UNDERLINE_SIZE,
    PG_FONT_SUB_X,
    PG_FONT_SUB_Y,
    PG_FONT_SUB_SX,
    PG_FONT_SUB_SY,
    PG_FONT_SUP_X,
    PG_FONT_SUP_Y,
    PG_FONT_SUP_SX,
    PG_FONT_SUP_SY,
};

// In order of USB HID keys with ASCII characters removed.
enum PgKey {
    PG_KEY_ENTER          = -0X28,
    PG_KEY_ESCAPE         = -0X29,
    PG_KEY_BACKSPACE      = -0X2A,
    PG_KEY_TAB            = -0X2B,

    PG_KEY_CAPS_LOCK      = -0X39,
    PG_KEY_F1             = -0X3A,
    PG_KEY_F2             = -0X3B,
    PG_KEY_F3             = -0X3C,
    PG_KEY_F4             = -0X3D,
    PG_KEY_F5             = -0X3E,
    PG_KEY_F6             = -0X3F,
    PG_KEY_F7             = -0X40,
    PG_KEY_F8             = -0X41,
    PG_KEY_F9             = -0X42,
    PG_KEY_F10            = -0X43,
    PG_KEY_F11            = -0X44,
    PG_KEY_F12            = -0X45,
    PG_KEY_PRINT_SCREEN   = -0X46,
    PG_KEY_SCROLL_LOCK    = -0X47,
    PG_KEY_PAUSE          = -0X48,
    PG_KEY_INSERT         = -0X49,
    PG_KEY_HOME           = -0X4A,
    PG_KEY_PAGE_UP        = -0X4B,
    PG_KEY_DELETE         = -0X4C,
    PG_KEY_END            = -0X4D,
    PG_KEY_PAGE_DOWN      = -0X4E,
    PG_KEY_RIGHT          = -0X4F,
    PG_KEY_LEFT           = -0X50,
    PG_KEY_DOWN           = -0X51,
    PG_KEY_UP             = -0X52,
    PG_KEY_NUM_LOCK       = -0X53,
    PG_KEY_KP_DIVIDE      = -0X54,
    PG_KEY_KP_MULTIPLY    = -0X55,
    PG_KEY_KP_SUBTRACT    = -0X56,
    PG_KEY_KP_ADD         = -0X57,
    PG_KEY_KP_ENTER       = -0X58,
    PG_KEY_KP_1           = -0X59,
    PG_KEY_KP_2           = -0X5A,
    PG_KEY_KP_3           = -0X5B,
    PG_KEY_KP_4           = -0X5C,
    PG_KEY_KP_5           = -0X5D,
    PG_KEY_KP_6           = -0X5E,
    PG_KEY_KP_7           = -0X5F,
    PG_KEY_KP_8           = -0X60,
    PG_KEY_KP_9           = -0X61,
    PG_KEY_KP_0           = -0X62,
    PG_KEY_KP_DECIMAL     = -0X63,
    PG_KEY_ISO_SLASH      = -0X64,
    PG_KEY_APPLICATION    = -0X65,
    PG_KEY_POWER          = -0X66,
    PG_KEY_KP_EQUAL       = -0X67,
    PG_KEY_F13            = -0X68,
    PG_KEY_F14            = -0X69,
    PG_KEY_F15            = -0X6A,
    PG_KEY_F16            = -0X6B,
    PG_KEY_F17            = -0X6C,
    PG_KEY_F18            = -0X6D,
    PG_KEY_F19            = -0X6E,
    PG_KEY_F20            = -0X6F,
    PG_KEY_F21            = -0X70,
    PG_KEY_F22            = -0X71,
    PG_KEY_F23            = -0X72,
    PG_KEY_F24            = -0X73,
    PG_KEY_EXECUTE        = -0x74,
    PG_KEY_HELP           = -0X75,
    PG_KEY_MENU           = -0X76,
    PG_KEY_SELECT         = -0X77,
    PG_KEY_STOP           = -0X78,
    PG_KEY_REDO           = -0X79,
    PG_KEY_UNDO           = -0X7A,
    PG_KEY_CUT            = -0X7B,
    PG_KEY_COPY           = -0X7C,
    PG_KEY_PASTE          = -0X7D,
    PG_KEY_FIND           = -0X7E,
    PG_KEY_MUTE           = -0X7F,
    PG_KEY_VOLUME_UP      = -0X80,
    PG_KEY_VOLUME_DOWN    = -0X81,

    PG_KEY_KP_SEPARATOR   = -0X85,

    PG_KEY_KANA_LONG      = -0X87,
    PG_KEY_KANA           = -0X88,
    PG_KEY_YEN            = -0X89,
    PG_KEY_CONVERT        = -0X8A,
    PG_KEY_NO_CONVERT     = -0X8B,

    PG_KEY_HALF_WIDTH     = -0X8C,
    PG_KEY_HANGUL         = -0X8E,
    PG_KEY_HANJA          = -0X8F,

    PG_KEY_HANKAKU        = -0X90,


    PG_KEY_SYS_REQ        = -0X9A,

    PG_KEY_CTRL_LEFT      = -0XE0,
    PG_KEY_SHIFT_LEFT     = -0XE1,
    PG_KEY_ALT_LEFT       = -0XE2,
    PG_KEY_WIN_LEFT       = -0XE3,
    PG_KEY_CTRL_RIGHT     = -0XE4,
    PG_KEY_SHIFT_RIGHT    = -0XE5,
    PG_KEY_ALT_RIGHT      = -0XE6,
    PG_KEY_WIN_RIGHT      = -0XE7,
};

enum PgMods {
    PG_MOD_SHIFT =  0x01,
    PG_MOD_CTRL =   0x02,
    PG_MOD_ALT =    0x04,
    PG_MOD_WIN =    0x08,
};


typedef enum PgPartType     PgPartType;
typedef enum PgPaintType    PgPaintType;
typedef enum PgColorSpace   PgColorSpace;
typedef enum PgLineCap      PgLineCap;
typedef enum PgFillRule     PgFillRule;
typedef enum PgTextPos      PgTextPos;
typedef enum PgFontProp     PgFontProp;
typedef enum PgKey          PgKey;
typedef enum PgMods         PgMods;


struct PgPt {
    float x;
    float y;
};

struct PgTM {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
};

struct PgPath {
    unsigned            nparts;
    PgPart              *parts;
    PgPt                cur;
};

struct PgPart {
    PgPartType          type;
    PgPt                pt[3];
};

struct PgPaint {
    PgPaintType         type;
    PgColorSpace        cspace;
    PgColor             colors[8];
    float               stops[8];
    unsigned            nstops;
    PgPt                a;
    PgPt                b;
    float               ra;
    float               rb;
};

struct PgState {
    PgTM                ctm;
    const PgPaint       *fill;
    const PgPaint       *stroke;
    const PgPaint       *clear;
    float               line_width;
    PgLineCap           line_cap;
    float               flatness;
    PgFillRule          fill_rule;
    float               clip_x;
    float               clip_y;
    float               clip_sx;
    float               clip_sy;
    PgTextPos           text_pos;
    bool                underline;
};

struct Pg {
    const PgCanvasImpl  *v;
    float               sx;
    float               sy;
    PgPath              *path;
    PgState             s;
    PgState             saved[16];
    unsigned            nsaved;
};

struct PgSubcanvas {
    Pg      _;
    Pg      *parent;
    float   x;
    float   y;
    float   sx;
    float   sy;
};

struct PgGroup {
    float   x;
    float   y;
    float   max_x;
    float   max_y;
    float   abs_x;
    float   abs_y;
    float   pad_x;
    float   pad_y;
    bool    horiz;
    Pg      *canvas;
    void    *data;
    void    (*end)(PgGroup *group);
};

struct PgFont {
    const PgFontImpl    *v;
    const uint8_t       *data;
    size_t              size;
    unsigned            index;
};

struct PgFamily {
    const char          *name;
    unsigned            nfaces;
    PgFace              *faces;
};

struct PgFace {
    const char          *family;
    const char          *style;
    const char          *path;
    unsigned            index;
    unsigned            width;
    unsigned            weight;
    bool                is_fixed;
    bool                is_italic;
    bool                is_serif;
    bool                is_sans_serif;
    uint8_t             style_class;
    uint8_t             style_subclass;
    uint8_t             panose[10];
};



// Extension.
struct PgCanvasImpl {
    void    (*clear)(Pg *g);
    void    (*fill)(Pg *g);
    void    (*stroke)(Pg *g);
    void    (*fill_stroke)(Pg *g);
    void    (*resize)(Pg *g, float width, float height);
    void    (*free)(Pg *g);
};

struct PgFontImpl {
    unsigned    (*get_glyph)(PgFont *font, uint32_t codepoint);
    void        (*glyph_path)(Pg *g, PgFont *font, float x, float y, uint32_t glyph);
    float       (*measure_glyph)(PgFont *font, uint32_t glyph);
    void        (*scale)(PgFont *font, float sx, float sy);
    float       (*number)(PgFont *font, PgFontProp id);
    int         (*_int)(PgFont *font, PgFontProp id);
    const char  *(*string)(PgFont *font, PgFontProp id);
    void        (*free)(PgFont *font);
};


void pg_shutdown(void);

// Canvas.

Pg *pg_opengl(float width, float height);
Pg *pg_subcanvas(Pg *parent, float x, float y, float sx, float sy);
void pg_free(Pg *g);

void pg_resize(Pg *g, float width, float height);

bool pg_restore(Pg *g);
bool pg_save(Pg *g);
void pg_reset_state(Pg *g);

void pg_reset_path(Pg *g);
void pg_move(Pg *g, float x, float y);
void pg_line(Pg *g, float x, float y);
void pg_rline(Pg *g, float x, float y);
void pg_curve3(Pg *g, float bx, float by, float cx, float cy);
void pg_curve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
void pg_rcurve3(Pg *g, float bx, float by, float cx, float cy);
void pg_rcurve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
void pg_rectangle(Pg *g, float x, float y, float sx, float sy);
void pg_rounded(Pg *g, float x, float y, float sx, float sy, float rx, float ry);
void pg_close(Pg *g);

void pg_clear(Pg *g);
void pg_fill(Pg *g);
void pg_stroke(Pg *g);
void pg_fill_stroke(Pg *g);

void pg_identity(Pg *g);
void pg_translate(Pg *g, float x, float y);
void pg_rotate(Pg *g, float rads);
void pg_scale(Pg *g, float x, float y);

void pg_set_fill(Pg *g, const PgPaint *paint);
void pg_set_stroke(Pg *g, const PgPaint *paint);
void pg_set_clear(Pg *g, const PgPaint *paint);
void pg_set_line_width(Pg *g, float line_width);
void pg_set_line_cap(Pg *g, PgLineCap line_cap);
void pg_set_flatness(Pg *g, float flatness);
void pg_set_fill_rule(Pg *g, PgFillRule fill_rule);
void pg_set_clip(Pg *g, float x, float y, float sx, float sy);
void pg_set_text_pos(Pg *g, PgTextPos text_pos);
void pg_set_underline(Pg *g, bool underline);

const PgPaint *pg_get_fill(Pg *g);
const PgPaint *pg_get_stroke(Pg *g);
const PgPaint *pg_get_clear(Pg *g);
float pg_get_line_width(Pg *g);
PgLineCap pg_get_line_cap(Pg *g);
float pg_get_flatness(Pg *g);
PgFillRule pg_get_fill_rule(Pg *g);
PgPt pg_get_clip_start(Pg *g);
PgPt pg_get_clip_size(Pg *g);
PgTextPos pg_get_text_pos(Pg *g);
bool pg_get_underline(Pg *g);


Pg pg_init_canvas(const PgCanvasImpl *v, float width, float height);



// Toolkit.
bool pg_init_tk(void);

Pg *pg_window(unsigned width, unsigned height, const char *title);
bool pg_event(void);

PgPt pg_dpi(void);
float pg_pad(void);
void pg_redraw(void);
void pg_update(void);
Pg* pg_root_canvas(void);

// Device status.
bool pg_should_activate(void);
PgPt pg_mouse_at(void);
unsigned pg_mouse_buttons(void);
float pg_mouse_wheel(void);
int32_t pg_key(void);
void pg_set_key(int32_t key);
unsigned pg_mod_keys(void);

// Standard controls.
bool pg_checkbox(const char *text, bool *checked);
bool pg_button(const char *text);
void pg_label(const char *text);
void pg_hslider(float width, float *val);
void pg_vslider(float height, float *val);
bool pg_item(float sx, const char *text, bool selected);
bool pg_dropdown(float sx, const char *text, bool *open);
void pg_vscroll(float sx, float sy, float total_y, float *vscroll);
bool pg_textbox(float width, char *text, size_t limit);

Pg* pg_ctrl(float sx, float sy);
void pg_group(bool horiz);
void pg_end_group(void);

PgFont* pg_font(void);

PgPt pg_ctrl_at(void);
PgPt pg_ctrl_size(void);
Pg* pg_ctrl_canvas(void);
unsigned pg_ctrl_id(void);
bool pg_is_active(void);
bool pg_is_focused(void);
bool pg_is_mouse_over(void);

void pg_group_clip(float x, float y, float sx, float sy);
void pg_set_group_pad(float x, float y);
void pg_set_group_cleanup(void subroutine(PgGroup *group), void *data);

unsigned pg_get_active(void);
void pg_set_active(unsigned id);
unsigned pg_get_focused(void);
void pg_set_focused(unsigned id);



// Paths.
PgPath *pg_path(void);
unsigned pg_partcount(PgPartType type);
void pg_path_close(PgPath *path);
void pg_path_curve3(PgPath *path, float bx, float by, float cx, float cy);
void pg_path_curve4(PgPath *path, float bx, float by, float cx, float cy, float dx, float dy);
void pg_path_free(PgPath *path);
void pg_path_line(PgPath *path, float x, float y);
void pg_path_rline(PgPath *path, float x, float y);
void pg_path_move(PgPath *path, float x, float y);
void pg_path_rcurve3(PgPath *path, float bx, float by, float cx, float cy);
void pg_path_rcurve4(PgPath *path, float bx, float by, float cx, float cy, float dx, float dy);
void pg_path_rectangle(PgPath *path, float x, float y, float sx, float sy);
void pg_path_rounded(PgPath *path, float x, float y, float sx, float sy, float rx, float ry);
void pg_path_reset(PgPath *path);





// Paint.
PgPaint pg_linear(PgColorSpace cspace, float ax, float ay, float bx, float by);
PgPaint pg_solid(PgColorSpace cspace, float x, float y, float z, float a);
void pg_add_stop(PgPaint *paint, float t, float x, float y, float z, float a);




// Font.

PgFont *pg_find_font(const char *family, unsigned weight, bool italic);
PgFamily *pg_list_fonts(void);

PgFont *pg_scale_font(PgFont *font, float sx, float sy);
float pg_font_height(PgFont *font);

void pg_free_font(PgFont *font);

float pg_char_path(Pg *g, PgFont *font, float x, float y, uint32_t codepoint);
float pg_chars_path(Pg *g, PgFont *font, float x, float y, const char *str, size_t nbytes);
float pg_glyph_path(Pg *g, PgFont *font, float x, float y, uint32_t glyph);
float pg_string_path(Pg *g, PgFont *font, float x, float y, const char *str);
float pg_printf(Pg *g, PgFont *font, float x, float y, const char *str, ...);
float pg_vprintf(Pg *g, PgFont *font, float x, float y, const char *str, va_list ap);

float pg_measure_char(PgFont *font, uint32_t codepoint);
float pg_measure_chars(PgFont *font, const char *str, size_t nbytes);
float pg_measure_glyph(PgFont *font, uint32_t glyph);
float pg_measure_string(PgFont *font, const char *str);

unsigned pg_fit_chars(PgFont *font, const char *s, size_t nbytes, float width);
unsigned pg_fit_string(PgFont *font, const char *str, float width);

unsigned pg_get_glyph(PgFont *font, uint32_t codepoint);

const char *pg_font_string(PgFont *font, PgFontProp id);
float pg_font_number(PgFont *font, PgFontProp id);
int pg_font_int(PgFont *font, PgFontProp id);

PgFont *pg_open_font(const uint8_t *data, size_t size, unsigned index);
PgFont *pg_open_font_file(const char *path, unsigned index);
PgFont *pg_open_otf_font(const uint8_t *data, size_t size, unsigned index);

PgFont pg_init_font(const PgFontImpl *v, const uint8_t *data, size_t size, unsigned index);



// Transform Matrix Manipulation.
PgPt pg_apply_tm(PgTM ctm, PgPt p);
PgTM pg_mul_tm(PgTM x, PgTM y);
PgTM pg_ident_tm(void);
PgTM pg_translate_tm(PgTM m, float x, float y);
PgTM pg_scale_tm(PgTM m, float x, float y);
PgTM pg_rotate_tm(PgTM m, float rad);



// Point and Vector Manipulation.
#define PgPt(X, Y)              ((PgPt) { (X), (Y) })


// UTF-8.


static
inline
size_t
pg_utf8_nbytes(uint32_t codepoint)
{
    if (codepoint < 0x80)
        return 1;

    else if (codepoint < 0x0800)
        return 2;

    else if (codepoint < 0x10000)
        return 3;

    else if (codepoint < 0x10ffff)
        return 4;

    return 0;
}

static
inline
size_t
pg_utf8_following(uint8_t lead)
{
    return  lead < 0x80? 0:
            lead < 0xd0? 1:
            lead < 0xf0? 2:
                         3;
}


static
inline
uint32_t
pg_read_utf8(const char **stringp, const char *limitp)
{
    const uint8_t   *in = (const uint8_t*) *stringp;
    const uint8_t   *limit = (const uint8_t*) limitp;

    uint32_t    c = *in++;
    size_t      nbytes =    c < 0x80? 0:
                            c < 0xd0? 1:
                            c < 0xf0? 2:
                                      3;
    uint32_t    min =       c < 0x80? 0x00:
                            c < 0xd0? 0x80:
                            c < 0xf0? 0x800:
                                      0x10000;
    uint32_t    mask =      c < 0x80? 0x7f:
                            c < 0xd0? 0x07ff:
                            c < 0xf0? 0xffff:
                                      0x10ffff;
    unsigned    got = 0;

    while (in < limit && (*in & 0xc0) == 0x80) {
        c = (c << 6) + (*in++ & 0x3f);
        got++;
    }

    *stringp = (const char*) in;

    if (got != nbytes)
        return 0xfffd;

    if (c < min)
        return 0xfffd;

    return c & mask;
}


static
inline
const char*
pg_utf8_start(const char *str, const char *start, const char *limit)
{
    if (str >= limit)
        return limit;

    if (str < start)
        return start;

    while (str > start && (*str & 0xc0) == 0x80)
        str--;

    return str;
}


static
inline
uint32_t
pg_rev_read_utf8(const char **in, const char *start, const char *limit)
{
    *in = pg_utf8_start(*in, start, limit);
    return pg_read_utf8((const char*[]) { *in }, limit);
}


static
inline
char*
pg_write_utf8(char *outp, char *limitp, uint32_t c)
{
    uint8_t *out = (uint8_t*) outp;
    uint8_t *limit = (uint8_t*) limitp;

    if (c < 0x80)
        *out++ = (uint8_t) c;

    else if (c < 0x0800) {
        if (out + 2 < limit) {
            *out++ = (uint8_t) (0xc0 + (c >> 6));
            *out++ = (uint8_t) (0x80 + (c & 0x3f));
        }
    }

    else if (c < 0x10000) {
        if (out + 3 < limit) {
            *out++ = (uint8_t) (0xe0 + (c >> 12));
            *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
            *out++ = (uint8_t) (0x80 + (c & 0x3f));
        }
    }

    else if (c < 0x10ffff) {
        if (out + 4 < limit) {
            *out++ = (uint8_t) (0xe0 + (c >> 18));
            *out++ = (uint8_t) (0x80 + (c >> 12 & 0x3f));
            *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
            *out++ = (uint8_t) (0x80 + (c & 0x3f));
        }
    }

    return (char*) out;
}


