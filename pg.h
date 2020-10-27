#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Pg           Pg;         // Canvas
typedef struct Pgcolor      Pgcolor;    // Colour
typedef struct Pgpt         Pgpt;       // Point
typedef struct Pgrect       Pgrect;     // Rectangle (point and size)
typedef struct Pgmat        Pgmat;      // Transform Matrix (2D)
typedef struct Pgpath       Pgpath;     // Path
typedef struct Pgpart       Pgpart;     // Part of path
typedef struct Pgpaint      Pgpaint;    // Paint
typedef struct Pgfont       Pgfont;     // Font
typedef struct Pgface       Pgface;     // Description of font face
typedef struct Pgfamily     Pgfamily;   // Description of font family


typedef enum {
    PG_PART_MOVE,
    PG_PART_LINE,
    PG_PART_CURVE3,
    PG_PART_CURVE4,
    PG_PART_CLOSE,
} Pgform;

typedef enum {
    PG_SOLID_PAINT,
    PG_LINEAR_PAINT,
} Pgpaint_type;

typedef enum {
    PG_SRGB,
    PG_LCHAB,
    PG_LAB,
    PG_XYZ,
} Pgcolorspace;

typedef enum {
    PG_BUTT_CAP,
    PG_SQUARE_CAP,
} Pgline_cap;

typedef enum {
    PG_NONZERO_RULE,
    PG_EVEN_ODD_RULE,
} Pgfill_rule;

typedef enum {
    PG_TEXT_POS_TOP,
    PG_TEXT_POS_BOTTOM,
    PG_TEXT_POS_BASELINE,
    PG_TEXT_POS_CENTER,
} Pgtext_pos;

typedef enum {
    PG_FONT_FORMAT,         // String for file format. e.g. "TTF", "CFF"
    PG_FONT_INDEX,          // Index used to create this font.
    PG_FONT_NFONTS,         // Number of fonts in the same file.
    PG_FONT_FAMILY,         // Name of the family.
    PG_FONT_STYLE,          // Self-described style. (e.g. "Bold", "Oblique")
    PG_FONT_FULL_NAME,      // Self-described full name with family and style.
    PG_FONT_FIXED,          // 1=Fixed-Pitched (monospaced); 0=Proportional
    PG_FONT_WEIGHT,         // Weight of font: 0-999. 400=Regular 700=Bold
    PG_FONT_WIDTH_CLASS,    // Width class: 0-9. 5=Normal
    PG_FONT_ANGLE,          // Angle of italics.
    PG_FONT_PANOSE,         // 10-number PANOSE classification for font.
    PG_FONT_NGLYPHS,        // Number of glyphs.
    PG_FONT_EM,             // Height of EM square (scaled).
    PG_FONT_AVG_WIDTH,      // Average character width (scaled).
    PG_FONT_ASCENDER,       // Distance from ascender to baseline (scaled).
    PG_FONT_DESCENDER,      // Distance from descender to baseline (scaled).
    PG_FONT_LINEGAP,        // Distance from ascender to top of EM (scaled).
    PG_FONT_XHEIGHT,        // Distance from top of 'x' to baseline (scaled).
    PG_FONT_CAPHEIGHT,      // Distance from top of capital to baseline (scaled).
    PG_FONT_SUB_X,          // Subscript square.
    PG_FONT_SUB_Y,
    PG_FONT_SUB_SX,
    PG_FONT_SUB_SY,
    PG_FONT_SUP_X,          // Superscript square.
    PG_FONT_SUP_Y,
    PG_FONT_SUP_SX,
    PG_FONT_SUP_SY,
} Pgfont_prop;


struct Pgcolor { float x, y, z, a; };
struct Pgpt { float x, y; };
struct Pgrect { Pgpt p, size; };
struct Pgmat { float a, b, c, d, e, f; };

struct Pgpath {
    unsigned    nparts;
    Pgpart      *parts;
};

struct Pgpart {
    Pgform      form;
    Pgpt        pt[3];
};

struct Pgpaint {
    Pgpaint_type    type;
    Pgcolorspace    cspace;
    Pgcolor         colours[8];
    float           stops[8];
    unsigned        nstops;
    Pgpt            a;
    Pgpt            b;
    float           ra;
    float           rb;
};

struct Pgfamily {
    const char  *name;
    unsigned    nfaces;
    Pgface      *faces;
};

struct Pgface {
    const char  *family;
    const char  *style;
    const char  *path;
    unsigned    index;
    unsigned    width;
    unsigned    weight;
    bool        fixed;
    bool        sloped;
    char        panose[10];
};



// Extension.
typedef struct {
    void    *(*init)(Pg *g);
    void    (*free)(Pg *g);
    void    (*resize)(Pg *g, unsigned width, unsigned height);
    void    (*clear)(Pg *g, Pgpaint paint);
    void    (*fill)(Pg *g);
    void    (*stroke)(Pg *g);
    void    (*fill_stroke)(Pg *g);
} Pgcanvas_methods;

typedef struct {
    void        *(*init)(Pgfont *font, const uint8_t *data, size_t size, unsigned index);
    void        (*free)(Pgfont *font);
    float       (*propf)(Pgfont *font, Pgfont_prop id);
    const char  *(*props)(Pgfont *font, Pgfont_prop id);
    void        (*glyph_path)(Pg *g, Pgfont *font, Pgpt p, unsigned glyph);
    Pgpt        (*measure_glyph)(Pgfont *font, unsigned glyph);
} Pgfont_methods;



// Canvas.
Pg *pg_opengl_canvas(unsigned width, unsigned height);

Pg  *pg_clear(Pg *g, Pgpaint paint);
Pg  *pg_fill(Pg *g);
Pg  *pg_stroke(Pg *g);
Pg  *pg_fill_stroke(Pg *g);

void pg_free_canvas(Pg *g);
Pg  *pg_resize_canvas(Pg *g, unsigned width, unsigned height);
Pgpt pg_get_size(Pg *g);
Pgpath pg_get_path(Pg *g);

void *pg_get_canvas_impl(const Pg *g);
Pg *pg_new_canvas(const Pgcanvas_methods *v, unsigned width, unsigned height);




// Paths.
Pgpt pg_cur(Pg *g);
Pg *pg_move_to(Pg *g, float x, float y);
Pg *pg_move_to_pt(Pg *g, Pgpt p);
Pg *pg_rel_move_to(Pg *g, float x, float y);
Pg *pg_rel_move_to_pt(Pg *g, Pgpt p);
Pg *pg_line_to(Pg *g, float x, float y);
Pg *pg_line_to_pt(Pg *g, Pgpt p);
Pg *pg_rel_line_to(Pg *g, float x, float y);
Pg *pg_rel_line_to_pt(Pg *g, Pgpt p);
Pg *pg_curve3_to(Pg *g, float bx, float by, float cx, float cy);
Pg *pg_curve3_to_pt(Pg *g, Pgpt b, Pgpt c);
Pg *pg_rel_curve3_to(Pg *g, float bx, float by, float cx, float cy);
Pg *pg_rel_curve3_to_pt(Pg *g, Pgpt b, Pgpt c);
Pg *pg_curve4_to(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
Pg *pg_curve4_to_pt(Pg *g, Pgpt b, Pgpt c, Pgpt d);
Pg *pg_rel_curve4_to(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
Pg *pg_rel_curve4_to_pt(Pg *g, Pgpt b, Pgpt c, Pgpt d);
Pg *pg_close_path(Pg *g);
Pg *pg_clear_path(Pg *g);
Pg *pg_rect_path(Pg *g, float x, float y, float sx, float sy);
Pg *pg_rect_path_abs(Pg *g, float ax, float ay, float bx, float by);
Pg *pg_rect_path_pt(Pg *g, Pgrect r);
Pg *pg_rrect_path(Pg *g, float x, float y, float sx, float sy, float rad);
Pg *pg_rrect_path_abs(Pg *g, float ax, float ay, float bx, float by, float rad);
Pg *pg_rrect_path_pt(Pg *g, Pgrect r, float rad);



// CTM.
Pg *pg_ctm_identity(Pg *g);
Pg *pg_ctm_translate(Pg *g, float x, float y);
Pg *pg_ctm_translate_pt(Pg *g, Pgpt v);
Pg *pg_ctm_scale(Pg *g, float x, float y);
Pg *pg_ctm_scale_pt(Pg *g, Pgpt v);
Pg *pg_ctm_rotate(Pg *g, float rads);



// Paint.
Pgpaint pg_solid_color(Pgcolorspace cspace, Pgcolor colour);
Pgpaint pg_solid(Pgcolorspace cspace, float x, float y, float z, float a);
Pgpaint pg_linear_pt(Pgcolorspace cspace, Pgpt a, Pgpt b);
Pgpaint pg_linear(Pgcolorspace cspace, float ax, float ay, float bx, float by);
Pgpaint *pg_add_stop_color(Pgpaint *paint, float t, Pgcolor colour);
Pgpaint *pg_add_stop(Pgpaint *paint, float t, float x, float y, float z, float a);
Pgpaint *pg_set_colorspace(Pgpaint *paint, Pgcolorspace cspace);



// State.
Pg *pg_save(Pg *g);
Pg *pg_restore(Pg *g);
Pgrect pg_get_clip(const Pg *g);
Pgmat pg_get_ctm(const Pg *g);
Pgpaint pg_get_fill(const Pg *g);
Pgpaint pg_get_stroke(const Pg *g);
float pg_get_line_width(const Pg *g);
Pgline_cap pg_get_line_cap(const Pg *g);
float pg_get_flatness(const Pg *g);
Pgfill_rule pg_get_fill_rule(const Pg *g);
Pgtext_pos pg_get_text_pos(const Pg *g);
Pg *pg_set_ctm(Pg *g, Pgmat ctm);
Pg *pg_set_clip(Pg *g, float x, float y, float sx, float sy);
Pg *pg_set_clip_abs(Pg *g, float ax, float ay, float bx, float by);
Pg *pg_set_clip_rect(Pg *g, Pgrect clip);
Pg *pg_set_fill_color(Pg *g, Pgcolorspace cspace, Pgcolor colour);
Pg *pg_set_fill(Pg *g, Pgpaint paint);
Pg *pg_set_stroke_color(Pg *g, Pgcolorspace cspace, Pgcolor colour);
Pg *pg_set_stroke(Pg *g, Pgpaint paint);
Pg *pg_set_line_width(Pg *g, float line_width);
Pg *pg_set_line_cap(Pg *g, Pgline_cap line_cap);
Pg *pg_set_flatness(Pg *g, float flatness);
Pg *pg_set_fill_rule(Pg *g, Pgfill_rule winding);
Pg *pg_set_text_pos(Pg *g, Pgtext_pos pos);



// Font.
Pgfamily *pg_list_fonts();
Pgfont *pg_find_font(const char *family, unsigned weight, bool sloped);
void pg_free_font(Pgfont *font);
Pgfont *pg_scale_font(Pgfont *font, float sx, float sy);

float pg_get_font_height(Pgfont *font);


Pgpt pg_glyph_path(Pg *g, Pgfont *font, Pgpt p, unsigned glyph);
Pgpt pg_char_path(Pg *g, Pgfont *font, Pgpt p, unsigned codepoint);
Pgpt pg_chars_path(Pg *g, Pgfont *font, Pgpt p, const char *s, unsigned n);
Pgpt pg_string_path(Pg *g, Pgfont *font, Pgpt p, const char *str);
Pgpt pg_vprintf(Pg *g, Pgfont *font, Pgpt p, const char *str, va_list ap);
Pgpt pg_printf(Pg *g, Pgfont *font, Pgpt p, const char *str, ...);
Pgpt pg_measure_glyph(Pgfont *font, unsigned glyph);
Pgpt pg_measure_char(Pgfont *font, unsigned codepoint);
Pgpt pg_measure_chars(Pgfont *font, const char *s, unsigned n);
Pgpt pg_measure_string(Pgfont *font, const char *str);
unsigned pg_fit_chars(Pgfont *font, const char *s, unsigned n, float width);
unsigned pg_fit_string(Pgfont *font, const char *str, float width);

unsigned pg_get_glyph(Pgfont *font, unsigned codepoint);
float pg_font_prop_float(Pgfont *font, Pgfont_prop id);
int pg_font_prop_int(Pgfont *font, Pgfont_prop id);
const char *pg_font_prop_string(Pgfont *font, Pgfont_prop id);

Pgfont *pg_open_font(const uint8_t *data, size_t size, unsigned index);
Pgfont *pg_open_font_file(const char *path, unsigned index);
Pgfont *pg_open_otf_font(const uint8_t *data, size_t size, unsigned index);
Pgfont *pg_new_font(const Pgfont_methods *v, const uint8_t *data, size_t size, unsigned index);
Pgfont *pg_init_font(Pgfont *font, float units, unsigned nglyphs, const uint16_t cmap[65536]);
void *pg_get_font_impl(const Pgfont *font);
Pgpt pg_get_font_scale(Pgfont *font);
unsigned pg_get_nglyphs(Pgfont *font);
float pg_get_em_units(Pgfont *font);



// Transform Matrix Manipulation.
static inline Pgpt pg_apply_mat(Pgmat ctm, Pgpt p);
static inline Pgmat pg_mul_mat(Pgmat x, Pgmat y);
static inline Pgmat pg_ident_mat();
static inline Pgmat pg_translate_mat(Pgmat m, float x, float y);
static inline Pgmat pg_scale_mat(Pgmat m, float x, float y);
static inline Pgmat pg_rotate_mat(Pgmat m, float rad);



// Colour conversion.
Pgcolor pg_lch_to_lab(Pgcolor lch);
Pgcolor pg_lab_to_xyz(Pgcolor lab);
Pgcolor pg_xyz_to_rgb(Pgcolor xyz);
Pgcolor pg_gamma(Pgcolor rgb);
Pgcolor pg_convert_color(Pgcolorspace cspace, Pgcolor colour);




// Point and Vector Manipulation.
static inline Pgpt pg_pt(float x, float y);
static inline Pgpt pg_zero_pt();
static inline Pgrect pg_empty_rect();
static inline Pgrect pg_rect_pt(Pgpt p, Pgpt size);
static inline Pgrect pg_rect(float x, float y, float sx, float sy);
static inline Pgrect pg_rect_abs(float ax, float ay, float bx, float by);
static inline bool pg_pt_in_rect(Pgrect r, Pgpt p);
static inline Pgpt pg_add_pts(Pgpt a, Pgpt b);
static inline Pgpt pg_sub_pts(Pgpt a, Pgpt b);
static inline Pgpt pg_mul_pts(Pgpt a, Pgpt b);
static inline Pgpt pg_div_pts(Pgpt a, Pgpt b);
static inline Pgpt pg_add_pt(Pgpt a, float x, float y);
static inline Pgpt pg_sub_pt(Pgpt a, float x, float y);
static inline Pgpt pg_mul_pt(Pgpt a, float x, float y);
static inline Pgpt pg_div_pt(Pgpt a, float x, float y);
static inline Pgpt pg_scale_pt(Pgpt p, float s);
static inline Pgpt pg_mid_pt(Pgpt a, Pgpt b);
static inline float pg_dist(Pgpt a, Pgpt b);
static inline Pgpt pg_normalize(Pgpt p);
static inline float pg_dot(Pgpt a, Pgpt b);



// UTF-8.
static inline uint32_t pg_read_utf8(const uint8_t **in);
static inline uint8_t *pg_write_utf8(uint8_t *out, uint32_t c);




static inline Pgpt pg_pt(float x, float y) {
    return (Pgpt) { x, y };
}

static inline Pgpt pg_zero_pt() {
    return (Pgpt) { 0.0f, 0.0f };
}

static inline Pgrect pg_empty_rect() {
    return (Pgrect) { {0.0f, 0.0f}, {0.0f, 0.0f} };
}

static inline Pgrect pg_rect_pt(Pgpt p, Pgpt size) {
    return (Pgrect) { p, size };
}

static inline Pgrect pg_rect(float x, float y, float sx, float sy) {
    return (Pgrect) { {x, y}, {sx, sy} };
}

static inline Pgrect pg_rect_abs(float ax, float ay, float bx, float by) {
    return (Pgrect) { {ax, ay}, {bx - ax, by - ay} };
}

static inline bool pg_pt_in_rect(Pgrect r, Pgpt p) {
    float   x = p.x - r.p.x;
    float   y = p.y - r.p.y;
    return  (0.0f <= x && x <= r.size.x) && (0.0f <= y && y <= r.size.y);
}

static inline Pgpt pg_add_pts(Pgpt a, Pgpt b) {
    return pg_pt(a.x+b.x, a.y+b.y);
}

static inline Pgpt pg_sub_pts(Pgpt a, Pgpt b) {
    return pg_pt(a.x-b.x, a.y-b.y);
}

static inline Pgpt pg_mul_pts(Pgpt a, Pgpt b) {
    return pg_pt(a.x*b.x, a.y*b.y);
}

static inline Pgpt pg_div_pts(Pgpt a, Pgpt b) {
    return pg_pt(a.x/b.x, a.y/b.y);
}

static inline Pgpt pg_add_pt(Pgpt a, float x, float y) {
    return pg_pt(a.x+x, a.y+y);
}

static inline Pgpt pg_sub_pt(Pgpt a, float x, float y) {
    return pg_pt(a.x-x, a.y-y);
}

static inline Pgpt pg_mul_pt(Pgpt a, float x, float y) {
    return pg_pt(a.x*x, a.y*y);
}

static inline Pgpt pg_div_pt(Pgpt a, float x, float y) {
    return pg_pt(a.x/x, a.y/y);
}

static inline Pgpt pg_scale_pt(Pgpt p, float s) {
    return pg_pt(p.x * s, p.y * s);
}

static inline Pgpt pg_mid_pt(Pgpt a, Pgpt b) {
    return pg_pt((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
}

static inline float pg_dist(Pgpt a, Pgpt b) {
    float   dx = a.x - b.x;
    float   dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}

static inline Pgpt pg_normalize(Pgpt p) {
    float invmag = 1.0f / sqrtf(p.x * p.x + p.y * p.y);
    return pg_pt(p.x * invmag, p.y * invmag);
}

static inline float pg_dot(Pgpt a, Pgpt b) {
    return a.x * b.x + a.y * b.y;
}

static inline Pgpt pg_apply_mat(Pgmat ctm, Pgpt p) {
    return pg_pt(ctm.a * p.x + ctm.c * p.y + ctm.e,
                ctm.b * p.x + ctm.d * p.y + ctm.f);
}

static inline Pgmat pg_mul_mat(Pgmat x, Pgmat y) {
    return (Pgmat) {
        (x.a * y.a) + (x.b * y.c) + (0.0f * y.e),
        (x.a * y.b) + (x.b * y.d) + (0.0f * y.f),
        (x.c * y.a) + (x.d * y.c) + (0.0f * y.e),
        (x.c * y.b) + (x.d * y.d) + (0.0f * y.f),
        (x.e * y.a) + (x.f * y.c) + (1.0f * y.e),
        (x.e * y.b) + (x.f * y.d) + (1.0f * y.f),
    };
}

static inline Pgmat pg_ident_mat() {
    return (Pgmat) {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
}

static inline Pgmat pg_translate_mat(Pgmat m, float x, float y) {
    return pg_mul_mat(m, (Pgmat) {1.0f, 0.0f, 0.0f, 1.0f, x, y});
}

static inline Pgmat pg_scale_mat(Pgmat m, float x, float y) {
    return pg_mul_mat(m, (Pgmat) {x, 0.0f, 0.0f, y, 0.0f, 0.0f});
}

static inline Pgmat pg_rotate_mat(Pgmat m, float rad) {
    float   sinx = sinf(rad);
    float   cosx = cosf(rad);
    return pg_mul_mat(m, (Pgmat) { cosx, sinx, -sinx, cosx, 0, 0 });
}

static inline unsigned pg_read_utf8_tail(const uint8_t **in, unsigned tail, unsigned min) {
    unsigned c = *(*in)++;
    unsigned got;
    for (got = 0; (**in & 0xc0) == 0x80; got++)
        c = (c << 6) + (*(*in)++ & 0x3f);
    return got == tail && c >= min? c: 0xfffd;
}

static inline uint32_t pg_read_utf8(const uint8_t **in) {
    if (**in < 0x80)
        return *(*in)++;
    if (**in < 0xd0)
        return pg_read_utf8_tail(in, 1, 0x80) & 0x07ff;
    if (**in < 0xf0)
        return pg_read_utf8_tail(in, 2, 0x800) & 0xffff;
    return pg_read_utf8_tail(in, 3, 0x10000) & 0x10ffff;
}

static inline uint8_t *pg_write_utf8(uint8_t *out, uint32_t c) {
    if (c < 0x80)
        *out++ = (uint8_t) c;
    else if (c < 0x0800) {
        *out++ = (uint8_t) (0xc0 + (c >> 6));
        *out++ = (uint8_t) (0x80 + (c & 0x3f));
    } else if (c < 0x10000) {
        *out++ = (uint8_t) (0xe0 + (c >> 12));
        *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
        *out++ = (uint8_t) (0x80 + (c & 0x3f));
    } else if (c < 0x10ffff) {
        *out++ = (uint8_t) (0xe0 + (c >> 18));
        *out++ = (uint8_t) (0x80 + (c >> 12 & 0x3f));
        *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
        *out++ = (uint8_t) (0x80 + (c & 0x3f));
    }
    return out;
}
