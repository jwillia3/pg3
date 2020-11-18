#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef struct Pg           Pg;         // Canvas
typedef struct PgState      PgState;    // State of canvas
typedef struct PgColor      PgColor;    // Colour
typedef struct PgPt         PgPt;       // Point
typedef struct PgTM         PgTM;       // Transform Matrix (2D)
typedef struct PgPath       PgPath;     // Path
typedef struct PgPart       PgPart;     // Part of path
typedef struct PgPaint      PgPaint;    // Paint
typedef struct PgFont       PgFont;     // Font
typedef struct PgFace       PgFace;     // Description of font face
typedef struct PgFamily     PgFamily;   // Description of font family

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
    PGGK_ENTER          = -0X28,
    PGGK_ESCAPE         = -0X29,
    PGGK_BACKSPACE      = -0X2A,
    PGGK_TAB            = -0X2B,

    PGGK_CAPS_LOCK      = -0X39,
    PGGK_F1             = -0X3A,
    PGGK_F2             = -0X3B,
    PGGK_F3             = -0X3C,
    PGGK_F4             = -0X3D,
    PGGK_F5             = -0X3E,
    PGGK_F6             = -0X3F,
    PGGK_F7             = -0X40,
    PGGK_F8             = -0X41,
    PGGK_F9             = -0X42,
    PGGK_F10            = -0X43,
    PGGK_F11            = -0X44,
    PGGK_F12            = -0X45,
    PGGK_PRINT_SCREEN   = -0X46,
    PGGK_SCROLL_LOCK    = -0X47,
    PGGK_PAUSE          = -0X48,
    PGGK_INSERT         = -0X49,
    PGGK_HOME           = -0X4A,
    PGGK_PAGE_UP        = -0X4B,
    PGGK_DELETE         = -0X4C,
    PGGK_END            = -0X4D,
    PGGK_PAGE_DOWN      = -0X4E,
    PGGK_RIGHT          = -0X4F,
    PGGK_LEFT           = -0X50,
    PGGK_DOWN           = -0X51,
    PGGK_UP             = -0X52,
    PGGK_NUM_LOCK       = -0X53,
    PGGK_KP_DIVIDE      = -0X54,
    PGGK_KP_MULTIPLY    = -0X55,
    PGGK_KP_SUBTRACT    = -0X56,
    PGGK_KP_ADD         = -0X57,
    PGGK_KP_ENTER       = -0X58,
    PGGK_KP_1           = -0X59,
    PGGK_KP_2           = -0X5A,
    PGGK_KP_3           = -0X5B,
    PGGK_KP_4           = -0X5C,
    PGGK_KP_5           = -0X5D,
    PGGK_KP_6           = -0X5E,
    PGGK_KP_7           = -0X5F,
    PGGK_KP_8           = -0X60,
    PGGK_KP_9           = -0X61,
    PGGK_KP_0           = -0X62,
    PGGK_KP_DECIMAL     = -0X63,
    PGGK_ISO_SLASH      = -0X64,
    PGGK_APPLICATION    = -0X65,
    PGGK_POWER          = -0X66,
    PGGK_KP_EQUAL       = -0X67,
    PGGK_F13            = -0X68,
    PGGK_F14            = -0X69,
    PGGK_F15            = -0X6A,
    PGGK_F16            = -0X6B,
    PGGK_F17            = -0X6C,
    PGGK_F18            = -0X6D,
    PGGK_F19            = -0X6E,
    PGGK_F20            = -0X6F,
    PGGK_F21            = -0X70,
    PGGK_F22            = -0X71,
    PGGK_F23            = -0X72,
    PGGK_F24            = -0X73,
    PGGK_EXECUTE        = -0x74,
    PGGK_HELP           = -0X75,
    PGGK_MENU           = -0X76,
    PGGK_SELECT         = -0X77,
    PGGK_STOP           = -0X78,
    PGGK_REDO           = -0X79,
    PGGK_UNDO           = -0X7A,
    PGGK_CUT            = -0X7B,
    PGGK_COPY           = -0X7C,
    PGGK_PASTE          = -0X7D,
    PGGK_FIND           = -0X7E,
    PGGK_MUTE           = -0X7F,
    PGGK_VOLUME_UP      = -0X80,
    PGGK_VOLUME_DOWN    = -0X81,

    PGGK_KP_SEPARATOR   = -0X85,

    PGGK_KANA_LONG      = -0X87,
    PGGK_KANA           = -0X88,
    PGGK_YEN            = -0X89,
    PGGK_CONVERT        = -0X8A,
    PGGK_NO_CONVERT     = -0X8B,

    PGGK_HALF_WIDTH     = -0X8C,
    PGGK_HANGUL         = -0X8E,
    PGGK_HANJA          = -0X8F,

    PGGK_HANKAKU        = -0X90,


    PGGK_SYS_REQ        = -0X9A,

    PGGK_CTRL_LEFT      = -0XE0,
    PGGK_SHIFT_LEFT     = -0XE1,
    PGGK_ALT_LEFT       = -0XE2,
    PGGK_WIN_LEFT       = -0XE3,
    PGGK_CTRL_RIGHT     = -0XE4,
    PGGK_SHIFT_RIGHT    = -0XE5,
    PGGK_ALT_RIGHT      = -0XE6,
    PGGK_WIN_RIGHT      = -0XE7,
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
    PgPt                size;
    PgPath              *path;
    PgState             s;
    PgState             saved[16];
    unsigned            nsaved;
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
PgPt pg_dpi(void);
Pg *pg_window(unsigned width, unsigned height, const char *title);
bool pg_wait(void);
void pg_update(void);
PgPt pg_mouse_location(void);
unsigned pg_mouse_buttons(void);
int32_t pg_key(void);
unsigned pg_mod_keys(void);


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

static inline unsigned
pg_read_utf8_tail(const uint8_t **in, const uint8_t *limit, unsigned nbytes, unsigned min)
{
    unsigned c = *(*in)++;
    unsigned got = 0;

    while (*in < limit && (**in & 0xc0) == 0x80) {
        c = (c << 6) + (*(*in)++ & 0x3f);
        got++;
    }

    if (got != nbytes)
        return 0xfffd;

    if (c < min)
        return 0xfffd;

    return c;
}


static inline uint32_t
pg_read_utf8(const uint8_t **in, const uint8_t *limit)
{
    if (limit == 0)
        limit = *in + 6; // A UTF-8 character can be 6 octets long.

    if (**in < 0x80)
        return *(*in)++;

    if (**in < 0xd0)
        return pg_read_utf8_tail(in, limit, 1, 0x80) & 0x07ff;

    if (**in < 0xf0)
        return pg_read_utf8_tail(in, limit, 2, 0x800) & 0xffff;

    return pg_read_utf8_tail(in, limit, 3, 0x10000) & 0x10ffff;
}


static inline uint8_t*
pg_write_utf8(uint8_t *out, uint8_t *limit, uint32_t c)
{
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

    return out;
}


