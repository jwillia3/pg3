#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct Pg           Pg;


/*
    Colour.
    Represent colours in a certain colour space.
    A colour is represented by four values between 0.0 and 1.0.
    Colours are associated with a colour space through `PgPaint`.
    By default, the colour space is sRGB and the channels are (R, G, B, A).
    CIELCh is defined by CIE Publication 15.
      - The channels are (L*, C*, h, A)
      - L* is lightness (normally mapped from 0 to 100)
      - C* is chroma (normally mapped from 0 to 100)
      - h is hue (normally mapped from 0° to 360°)
    CIELAB is defined by CIE Publication 15.
    XYZ (CIE 1931) and is generally used as a device independent intermediate.
 */
typedef struct { float x, y, z, a; } Pgcolor;



/*
    Coordinates.
*/
typedef struct { float x, y; } Pgpt;
typedef struct { Pgpt a, b; } Pgrect;
typedef struct { float a, b, c, d, e, f; } Pgmat;

static inline Pgpt pgpt(float x, float y) { return (Pgpt) { x, y }; }
static inline Pgrect pgrect(Pgpt x, Pgpt y) { return (Pgrect) { x, y }; }
static inline Pgrect pgrectf(float ax, float ay, float bx, float by) {
    return pgrect(pgpt(ax, ay), pgpt(bx, by));
}



/*
    Paths.
    Paths define shapes that may subsequently be used to paint on a canvas.
    Paths generally behave like PostScript/PDF paths.
    Paths can be traversed by looping through the parts, `parts`.
    The number of parts is given in `n`.
*/
typedef enum {
    PG_PART_MOVE,
    PG_PART_LINE,
    PG_PART_CURVE3,
    PG_PART_CURVE4,
    PG_PART_CLOSE,
} Pgform;

typedef struct {
    Pgform  form;
    Pgpt    pt[3];
} Pgpart;

typedef struct {
    unsigned    n;
    Pgpart      *parts;
} Pgpath;

Pgpath  pgpath(Pg *g);
Pg      *pgmove(Pg *g, Pgpt p);
Pg      *pgmovef(Pg *g, float x, float y);
Pg      *pgline(Pg *g, Pgpt p);
Pg      *pglinef(Pg *g, float x, float y);
Pg      *pgcurve3(Pg *g, Pgpt b, Pgpt c);
Pg      *pgcurve3f(Pg *g, float bx, float by, float cx, float cy);
Pg      *pgcurve4(Pg *g, Pgpt b, Pgpt c, Pgpt d);
Pg      *pgcurve4f(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
Pg      *pgclose(Pg *g);
Pg      *pgreset_path(Pg *g);



/*
    Canvas Creation.
    User-defined canvases can be created by defining a struct where the
    first member is a `Pg` struct initialised with `pginit_canvas()`.
    The virtual method table defines how the canvas should handle events.
    Multiple members of the same canvas type generally share the same vtable.
*/

typedef enum { PG_BUTT_CAP, PG_SQUARE_CAP } Pglinecap;
typedef enum { PG_NONZERO_RULE, PG_EVEN_ODD_RULE } Pgfill;
typedef enum { PG_SOLID_PAINT, PG_LINEAR_PAINT, PG_RADIAL_PAINT } Pgpaint_type;
typedef enum { PG_SRGB, PG_LCHAB, PG_LAB, PG_XYZ  } Pgcolorspace;

typedef struct {
    Pgpaint_type    type;
    Pgcolorspace    cspace;
    Pgcolor         colours[8];
    float           stops[8];
    unsigned        nstops;
    Pgpt            a;
    Pgpt            b;
    float           ra;
    float           rb;
} Pgpaint;

typedef struct {
    void    (*free)(Pg *g);
    void    (*resize)(Pg *g, unsigned width, unsigned height);
    void    (*clear)(Pg *g, Pgpaint paint);
    void    (*fill)(Pg *g);
    void    (*stroke)(Pg *g);
    void    (*fillstroke)(Pg *g);
} Pgcanvas_methods;

typedef struct {
    Pgmat       ctm;
    Pgpaint     fill;
    Pgpaint     stroke;
    float       linewidth;
    Pglinecap   linecap;
    float       flatness;
    Pgfill      rule;
    Pgrect      clip;
} pgstate;

struct Pg {
    const Pgcanvas_methods *v;
    int         width;
    int         height;
    Pgpath      path;
    pgstate     s;
    pgstate     saved[16];
    unsigned    nsaved;
};

static inline Pg
pginit_canvas(const Pgcanvas_methods *v, unsigned width, unsigned height) {
    return (Pg) {
        .v = v,
        .width = width,
        .height = height,
        .path = {0, 0},
        .s = {
            .ctm = {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
            .fill = {
                .type = PG_SOLID_PAINT,
                .cspace = PG_SRGB,
                .colours[0] = {0.0f, 0.0f, 0.0f, 1.0f},
                .nstops = 1,
            },
            .stroke = {
                .type = PG_SOLID_PAINT,
                .cspace = PG_SRGB,
                .colours[0] = {0.0f, 0.0f, 0.0f, 1.0f},
                .nstops = 1,
            },
            .linewidth = 1.0f,
            .linecap = PG_BUTT_CAP,
            .flatness = 1.0f,
            .rule = PG_NONZERO_RULE,
            .clip = {{0.0f, 0.0f}, {(float) width, (float) height}},
        }
    };
}


/* Canvas Creation and Manipulation. */
Pg  *pgopengl_canvas(int width, int height);

void pgfree_canvas(Pg *g);
float pgwidth(Pg *g);
float pgheight(Pg *g);


/* Transform Matrix (TM) Manipulation. */
Pg  *pgident(Pg *g);
Pg  *pgtranslate(Pg *g, Pgpt v);
Pg  *pgtranslatef(Pg *g, float x, float y);
Pg  *pgscale(Pg *g, Pgpt v);
Pg  *pgscalef(Pg *g, float x, float y);
Pg  *pgrotate(Pg *g, float rads);


/* Drawing Operations. */
Pg  *pgclear(Pg *g, Pgpaint paint);
Pg  *pgresize(Pg *g, int width, int height);
Pg  *pgfill(Pg *g);
Pg  *pgstroke(Pg *g);
Pg  *pgfillstroke(Pg *g);


/* Paints. */
Pgpaint pgsolid(Pgcolor colour);
Pgpaint pgsolidf(float x, float y, float z, float a);
Pgpaint pglinear(Pgpt a, Pgpt b);
Pgpaint pglinearf(float ax, float ay, float bx, float by);
Pgpaint pgradial(Pgpt a, float ra, Pgpt b, float rb);
Pgpaint pgradialf(float ax, float ay, float ra, float bx, float by, float rb);
Pgpaint *pgadd_stop(Pgpaint *paint, float t, Pgcolor colour);
Pgpaint *pgadd_stopf(Pgpaint *paint, float t, float x, float y, float z, float a);
Pgpaint *pgset_colorspace(Pgpaint *paint, Pgcolorspace cspace);
Pgcolorspace pgset_default_colorspace(Pgcolorspace cspace);
Pgcolorspace pgget_default_colorspace();


/* State Manipulation. */
Pg          *pgsave(Pg *g);
Pg          *pgrestore(Pg *g);
Pgrect      pgget_clip(const Pg *g);
Pgmat       pgget_ctm(const Pg *g);
Pgpaint     pgget_fill(const Pg *g);
Pgpaint     pgget_stroke(const Pg *g);
float       pgget_linewidth(const Pg *g);
Pglinecap   pgget_linecap(const Pg *g);
float       pgget_flatness(const Pg *g);
Pgfill      pgget_fill_rule(const Pg *g);
Pg          *pgset_ctm(Pg *g, Pgmat ctm);
Pg          *pgset_clip(Pg *g, Pgrect clip);
Pg          *pgset_clipp(Pg *g, Pgpt a, Pgpt b);
Pg          *pgset_clipf(Pg *g, float ax, float ay, float bx, float by);
Pg          *pgset_fill_color(Pg *g, Pgcolor colour);
Pg          *pgset_fill(Pg *g, Pgpaint paint);
Pg          *pgset_stroke_color(Pg *g, Pgcolor colour);
Pg          *pgset_stroke(Pg *g, Pgpaint paint);
Pg          *pgset_line_width(Pg *g, float linewidth);
Pg          *pgset_line_cap(Pg *g, Pglinecap linecap);
Pg          *pgset_flatness(Pg *g, float flatness);
Pg          *pgset_fill_rule(Pg *g, Pgfill winding);



/* Fonts. */

typedef struct Pgfont Pgfont;

typedef enum {
    PG_FONT_FORMAT,         // String for file format. e.g. "TTF", "CFF"
    PG_FONT_INDEX,          // Index used to create this font.
    PG_FONT_NFONTS,         // Number of fonts in the same file.
    PG_FONT_FAMILY,         // Name of the family.
    PG_FONT_STYLE,          // Self-described style. (e.g. "Bold", "Oblique")
    PG_FONT_FULL_NAME,      // Self-described full name with family and style.
    PG_FONT_FIXED,          // 1=Fixed-Pitched (monospaced); 0=Proportional
    PG_FONT_WEIGHT,         // Weight of font: 0-999. 300=Regular 700=Bold
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
} Pgfontprop;

typedef struct {
    void        (*free)(Pgfont *font);
    float       (*propf)(Pgfont *font, Pgfontprop id);
    const char  *(*props)(Pgfont *font, Pgfontprop id);
    void        (*glyph)(Pg *g, Pgfont *font, Pgpt p, unsigned glyph);
    float       (*measureglyph)(Pgfont *font, unsigned glyph);
} Pgfont_methods;

struct Pgfont {
    const Pgfont_methods *v;

    const uint8_t   *data;          // Pointer to file content.
    size_t          size;           // Size of file.
    unsigned        nglyphs;        // Number of glyphs in the font.
    uint16_t        *cmap;          // Mapping of Unicode BMP to glyph ID.

    float           units;          // Units per EM.
    float           sx;             // Scale.
    float           sy;             // Scale.

    char            propbuf[256];   // Temporary string for pgprops().
};

typedef struct {
    const char  *family;
    const char  *style;
    const char  *path;
    unsigned    index;
    unsigned    width;
    unsigned    weight;
    bool        fixed;
    bool        sloped;
    char        panose[10];
} Pgface;

typedef struct {
    const char  *name;
    unsigned    nfaces;
    Pgface      *faces;
} Pgfamily;

static inline Pgfont
pginit_font(const Pgfont_methods *v,
            const void      *data,
            size_t          size,
            unsigned        nglyphs,
            uint16_t        cmap[65536],
            float           units)
{
    return (Pgfont) {
        .v = v,
        .data = data,
        .size = size,
        .nglyphs = nglyphs,
        .cmap = memcpy(malloc(65536 * sizeof *cmap), cmap, 65536 * sizeof *cmap),
        .sx = 1.0f,
        .sy = 1.0f,
        .units = units,
    };
}

Pgfamily    *pglist_fonts();
Pgfont      *pgfind_font(const char *family, unsigned weight, bool sloped);
Pgfont      *pgopen_font_file(const char *path, unsigned index);
Pgfont      *pgopen_font(const uint8_t *data, size_t size, unsigned index);
void        pgfree_font(Pgfont *font);
float       pgfontpropf(Pgfont *font, Pgfontprop id);
int         pgfontpropi(Pgfont *font, Pgfontprop id);
const char  *pgfontprops(Pgfont *font, Pgfontprop id);
Pgfont      *pgscale_font(Pgfont *font, float sx, float sy);

unsigned    pgget_glyph(Pgfont *font, unsigned codepoint);
Pgpt        pgdraw_glyph(Pg *g, Pgfont *font, Pgpt p, unsigned glyph);
Pgpt        pgdraw_char(Pg *g, Pgfont *font, Pgpt p, unsigned codepoint);
Pgpt        pgdraw_chars(Pg *g, Pgfont *font, Pgpt p, const char *s, unsigned n);
Pgpt        pgdraw_string(Pg *g, Pgfont *font, Pgpt p, const char *str);
Pgpt        pgvprintf(Pg *g, Pgfont *font, Pgpt p, const char *str, va_list ap);
Pgpt        pgprintf(Pg *g, Pgfont *font, Pgpt p, const char *str, ...);
float       pgmeasure_glyph(Pgfont *font, unsigned glyph);
float       pgmeasure_char(Pgfont *font, unsigned codepoint);
float       pgmeasure_chars(Pgfont *font, const char *s, unsigned n);
float       pgmeasure_string(Pgfont *font, const char *str);
