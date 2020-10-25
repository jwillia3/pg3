#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <pg.h>
#include <pgutil.h>
#include "pg.internal.h"

static Pgfamily     *allfamilies;
static Pgcolorspace default_colorspace;


static inline int stricmp(const char *ap, const char *bp) {
    const uint8_t *a = (const uint8_t *) ap;
    const uint8_t *b = (const uint8_t *) bp;
    while (*a && *b) {
        uint32_t ca = towlower(pgread_utf8(&a));
        uint32_t cb = towlower(pgread_utf8(&b));
        if (ca != cb)
            return ca < cb? -1: 1;
    }
    return *a? 1: *b? -1: 0;
}

void pgfree_canvas(Pg *g) {
    if (g) {
        if (g->v && g->v->free)
            g->v->free(g);
        free(g);
    }
}
Pg *pgresize(Pg *g, int width, int height) {
    if (g && width > 0 && height > 0) {
        if (g->v && g->v->resize)
            g->v->resize(g, width, height);
        g->width = width;
        g->height = height;
        g->s.clip = pgrectf(0.0f, 0.0f, (float) width, (float) height);
    }
    return g;
}
float pgwidth(Pg *g) { return g? (float) g->width: 0.0f; }
float pgheight(Pg *g) { return g? (float) g->height: 0.0f; }
Pgpath pgpath(Pg *g) { return g? g->path: (Pgpath) {0, 0}; }


/*
    Paints.
*/

Pgcolorspace pgset_default_colorspace(Pgcolorspace cspace) {
    Pgcolorspace old = default_colorspace;
    switch (cspace) {
    case PG_SRGB:
    case PG_LCHAB:
    case PG_LAB:
    case PG_XYZ:
        default_colorspace = cspace;
        break;
    }
    return old;
}
Pgcolorspace pgget_default_colorspace() {
    return default_colorspace;
}
Pgpaint pgsolid(Pgcolor colour) {
    return (Pgpaint) {
        .type = PG_SOLID_PAINT,
        .cspace = default_colorspace,
        .colours[0] = colour,
        .nstops = 1,
    };
}
Pgpaint pgsolidf(float x, float y, float z, float a) {
    return pgsolid((Pgcolor) {x, y, z, a});
}
Pgpaint pglinear(Pgpt a, Pgpt b) {
    return (Pgpaint) {
        .type = PG_LINEAR_PAINT,
        .cspace = default_colorspace,
        .colours[0] = (Pgcolor) {0.0f, 0.0f, 0.0f, 1.0f},
        .stops[0] = 0.0f,
        .nstops = 0,
        .a = a,
        .b = b,
    };
}
Pgpaint pglinearf(float ax, float ay, float bx, float by) {
    return pglinear(pgpt(ax, ay), pgpt(bx, by));
}
Pgpaint pgradial(Pgpt a, float ra, Pgpt b, float rb) {
    return (Pgpaint) {
        .type = PG_RADIAL_PAINT,
        .cspace = PG_SRGB,
        .colours[0] = (Pgcolor) {0.0f, 0.0f, 0.0f, 1.0f},
        .stops[0] = 0.0f,
        .nstops = 0,
        .a = a,
        .b = b,
        .ra = ra,
        .rb = rb,
    };
}
Pgpaint
pgradialf(float ax, float ay, float ra, float bx, float by, float rb) {
    return pgradial(pgpt(ax, ay), ra, pgpt(bx, by), rb);
}
Pgpaint *pgadd_stop(Pgpaint *paint, float t, Pgcolor colour) {
    if (paint) {
        float   last = paint->nstops
                        ? paint->stops[paint->nstops - 1]
                        : 0.0f;
        size_t  max = sizeof paint->stops / sizeof *paint->stops;
        if (paint->nstops < max && last <= t && t <= 1.0f) {
            // Add small distance if t is same as last.
            if (paint->nstops && last == t)
                t = nextafterf(last, last + 1.0f);
            paint->stops[paint->nstops] = t;
            paint->colours[paint->nstops] = colour;
            paint->nstops++;
        }
    }
    return paint;
}
Pgpaint *pgadd_stopf(Pgpaint *paint, float t, float x, float y, float z, float a) {
    return pgadd_stop(paint, t, (Pgcolor) {x, y, z, a});
}
Pgpaint *pgset_colorspace(Pgpaint *paint, Pgcolorspace cspace) {
    if (paint)
        paint->cspace = cspace;
    return paint;
}


/*
    State Manipulation.
*/

Pg *pgsave(Pg *g) {
    if (g && g->nsaved + 1 < sizeof g->saved / sizeof *g->saved)
        g->saved[g->nsaved++] = g->s;
    return g;
}
Pg *pgrestore(Pg *g) {
    if (g && g->nsaved)
        g->s = g->saved[--g->nsaved];
    return g;
}
Pgrect pgget_clip(const Pg *g) {
    return g? g->s.clip: pgrectf(0.0f, 0.0f, 0.0f, 0.0f);
}
Pgmat pgget_ctm(const Pg *g) {
    return g? g->s.ctm: pgident_mat();
}
Pg *pgset_clip(Pg *g, Pgrect clip) {
    if (g)
        g->s.clip = clip;
    return g;
}
Pg *pgset_clipp(Pg *g, Pgpt a, Pgpt b) {
    return pgset_clip(g, pgrect(a, b));
}
Pg *pgset_clipf(Pg *g, float ax, float ay, float bx, float by) {
    return pgset_clipp(g, pgpt(ax, ay), pgpt(bx, by));
}
Pg *pgset_ctm(Pg *g, Pgmat ctm) {
    if (g)
        g->s.ctm = ctm;
    return g;
}
Pgpaint pgget_fill(const Pg *g) {
    return g? g->s.fill: pgsolid((Pgcolor) {0.0f, 0.0f, 0.0f, 1.0f});
}
Pg *pgset_fill(Pg *g, Pgpaint paint) {
    if (g)
        g->s.fill = paint;
    return g;
}
Pg *pgset_fill_color(Pg *g, Pgcolor colour) {
    if (g)
        g->s.fill = pgsolid(colour);
    return g;
}
Pgpaint pgget_stroke(const Pg *g) {
    return g? g->s.stroke: (Pgpaint) {0};
}
Pg *pgset_stroke(Pg *g, Pgpaint paint) {
    if (g)
        g->s.stroke = paint;
    return g;
}
Pg *pgset_stroke_color(Pg *g, Pgcolor colour) {
    if (g)
        g->s.stroke = pgsolid(colour);
    return g;
}
float pgget_linewidth(const Pg *g) {
    return g? g->s.linewidth: 1.0f;
}
Pg *pgset_line_width(Pg *g, float linewidth) {
    if (g && linewidth > 0.0f)
        g->s.linewidth = linewidth;
    return g;
}
Pglinecap pgget_linecap(const Pg *g) {
    return g? g->s.linecap: PG_BUTT_CAP;
}
Pg *pgset_line_cap(Pg *g, Pglinecap linecap) {
    if (g && (linecap == PG_BUTT_CAP || linecap == PG_SQUARE_CAP))
        g->s.linecap = linecap;
    return g;
}
float pgget_flatness(const Pg *g) {
    return g? g->s.flatness: 1.0f;
}
Pg *pgset_flatness(Pg *g, float flatness) {
    if (g && flatness >= 0.0f && flatness <= 100.0f)
        g->s.flatness = flatness;
    return g;
}
Pgfill pgget_fill_rule(const Pg *g) {
    return g? g->s.rule: PG_NONZERO_RULE;
}
Pg *pgset_fill_rule(Pg *g, Pgfill rule) {
    if (g && (rule == PG_NONZERO_RULE || rule == PG_EVEN_ODD_RULE))
        g->s.rule = rule;
    return g;
}



/*
    Path Operations.
*/
static size_t pathcap(size_t n) {
    return (n + 31UL) & ~31UL;
}
static Pgform prevpart(Pgpath path) {
    return path.n? path.parts[path.n - 1].form: PG_PART_CLOSE;
}
static void addpt(Pgpath *path, Pgpart part) {
    if (path->n + 1 >= pathcap(path->n)) {
        size_t cap = pathcap(path->n + 1);
        path->parts = realloc(path->parts, cap * sizeof *path->parts);
    }
    path->parts[path->n++] = part;
}
Pg *pgmove(Pg *g, Pgpt p) {
    if (g)
        addpt(&g->path, (Pgpart) {PG_PART_MOVE, {p}});
    return g;
}
Pg *pgmovef(Pg *g, float x, float y) {
    return pgmove(g, pgpt(x, y));
}
Pg *pgline(Pg *g, Pgpt p) {
    if (g && prevpart(g->path) != PG_PART_CLOSE)
        addpt(&g->path, (Pgpart) {PG_PART_LINE, {p}});
    return g;
}
Pg *pglinef(Pg *g, float x, float y) {
    return pgline(g, pgpt(x, y));
}
Pg *pgcurve3(Pg *g, Pgpt b, Pgpt c) {
    if (g && prevpart(g->path) != PG_PART_CLOSE)
        addpt(&g->path, (Pgpart) {PG_PART_CURVE3, {b, c}});
    return g;
}
Pg *pgcurve3f(Pg *g, float bx, float by, float cx, float cy) {
    return pgcurve3(g, pgpt(bx, by), pgpt(cx, cy));
}
Pg *pgcurve4(Pg *g, Pgpt b, Pgpt c, Pgpt d) {
    if (g && prevpart(g->path) != PG_PART_CLOSE)
        addpt(&g->path, (Pgpart) {PG_PART_CURVE4, {b, c, d}});
    return g;
}
Pg *pgcurve4f(Pg *g, float bx, float by, float cx, float cy, float dx, float dy) {
    return pgcurve4(g, pgpt(bx, by), pgpt(cx, cy), pgpt(dx, dy));
}
Pg *pgclose(Pg *g) {
    if (g && prevpart(g->path) != PG_PART_CLOSE)
        addpt(&g->path, (Pgpart) {PG_PART_CLOSE, {{0.0f, 0.0f}}});
    return g;
}
Pg *pgreset_path(Pg *g) {
    if (g)
        g->path.n = 0;
    return g;
}



/*
    Current Transform Matrix (CTM) Manipulation.
*/
Pg *pgident(Pg *g) {
    if (g)
        g->s.ctm = pgident_mat();
    return g;
}
Pg *pgtranslate(Pg *g, Pgpt v) {
    if (g)
        g->s.ctm = pgtranslate_mat(g->s.ctm, v.x, v.y);
    return g;
}
Pg *pgtranslatef(Pg *g, float x, float y) {
    return pgtranslate(g, pgpt(x, y));
}
Pg *pgscale(Pg *g, Pgpt v) {
    if (g)
        g->s.ctm = pgscale_mat(g->s.ctm, v.x, v.y);
    return g;
}
Pg *pgscalef(Pg *g, float x, float y) {
    return pgscale(g, pgpt(x, y));
}
Pg *pgrotate(Pg *g, float rads) {
    if (g)
        g->s.ctm = pgrotate_mat(g->s.ctm, rads);
    return g;
}



/*
    Drawing Operations.
*/
Pg *pgclear(Pg *g, Pgpaint paint) {
    if (g && g->v && g->v->clear)
        g->v->clear(g, paint);
    return g;
}
Pg *pgfill(Pg *g) {
    if (g) {
        if (g->v && g->v->fill)
            g->v->fill(g);
        pgreset_path(g);
    }
    return g;
}
Pg *pgstroke(Pg *g) {
    if (g) {
        if (g->v && g->v->stroke)
            g->v->stroke(g);
        pgreset_path(g);
    }
    return g;
}
Pg *pgfillstroke(Pg *g) {
    if (g) {
        if (g->v && g->v->fillstroke)
            g->v->fillstroke(g);
        else {
            if (g->v && g->v->fill)
                g->v->fill(g);
            if (g->v && g->v->stroke)
                g->v->stroke(g);
        }
        pgreset_path(g);
    }
    return g;
}



/*
    Font Management.
*/

// Compare faces by (family, width, weight, sloped, style).
static int compareface(const void *ap, const void *bp) {
    const Pgface  *a = (const Pgface*) ap;
    const Pgface  *b = (const Pgface*) bp;
    int r;
    return (r = stricmp(a->family, b->family))? r:
           a->width != b->width?    (int) a->width - (int) b->width:
           a->weight != b->weight?  (int) a->weight - (int) b->weight:
           a->sloped != b->sloped?  (int) a->sloped - (int) b->sloped:
           (r = stricmp(a->style, b->style))? r:
           0;
}

Pgfamily *pglist_fonts() {

    if (allfamilies)
        return allfamilies;

    char        *queue[256];
    unsigned    nqueue = 0;
    char        **files = 0;
    unsigned    nfiles = 0;

    // Get all font files.
    nqueue = _pgget_font_dirs(queue);
    while (nqueue) {
        char    *dirname = queue[--nqueue];
        DIR     *dir = opendir(dirname);
        struct dirent *e;

        while (dir && (e = readdir(dir))) {
            char    path[FILENAME_MAX + 1];
            sprintf(path, "%s/%s", dirname, e->d_name);

            if (e->d_name[0] == '.') // Hidden.
                ;
            else if (e->d_type == DT_DIR) { // Directory.
                if (nqueue < 256)
                    queue[nqueue++] = strdup(path);
            }
            else {  // File.
                char ext[FILENAME_MAX];
                strcpy(ext, strrchr(path, '.')? strrchr(path, '.'): "");
                for (char *i = ext; *i; i++)
                    *i = (char) toupper(*i);

                bool is_otf =  !strcmp(ext, ".TTF") ||
                                !strcmp(ext, ".TTC") ||
                                !strcmp(ext, ".OTF");

                if (is_otf) {
                    files = realloc(files, ++nfiles * sizeof *files);
                    files[nfiles - 1] = strdup(path);
                }
            }
        }

        free(dirname);
    }


    // Get font properties.
    Pgface      *faces = 0;
    unsigned    nfaces = 0;
    Pgfont      *font;

    for (unsigned i = 0; i < nfiles; i++)
        if ((font = pgopen_font_file(files[i], 0))) {
            Pgface  f = {0};
            f.family = strdup(pgfontprops(font, PG_FONT_FAMILY));
            f.style = strdup(pgfontprops(font, PG_FONT_STYLE));
            f.path = files[i];
            f.index = (unsigned) pgfontpropi(font, PG_FONT_INDEX);
            f.width = (unsigned) pgfontpropi(font, PG_FONT_WIDTH_CLASS);
            f.weight = (unsigned) pgfontpropi(font, PG_FONT_WEIGHT);
            f.fixed = (unsigned) pgfontpropi(font, PG_FONT_FIXED);
            f.sloped = pgfontpropf(font, PG_FONT_ANGLE) != 0.0f;
            strcpy(f.panose, pgfontprops(font, PG_FONT_PANOSE));
            pgfree_font(font);

            faces = realloc(faces, (nfaces + 2) * sizeof *faces);
            faces[nfaces++] = f;
        }
    faces[nfaces] = (Pgface) { 0 };
    free(files);

    // Sort them and put them into families.
    qsort(faces, nfaces, sizeof *faces, compareface);

    // Group into families.
    Pgfamily    *families = 0;
    unsigned    nfamilies = 0;
    for (unsigned i = 0; i < nfaces; ) {
        unsigned first = i;
        while (i < nfaces && !strcmp(faces[i].family, faces[first].family))
            i++;

        unsigned    n = i - first;
        Pgface      *children = malloc((n + 1) * sizeof *faces);
        memcpy(children, faces + first, n * sizeof *faces);
        children[n] = (Pgface) { 0 };

        families = realloc(families, (nfamilies + 2) * sizeof *families);
        families[nfamilies++] = (Pgfamily) {faces[first].family, n, children};
    }
    families[nfamilies] = (Pgfamily) { 0 };
    free(faces);

    allfamilies = families;
    return families;
}

Pgfont *pgfind_font(const char *family, unsigned weight, bool sloped) {
    Pgfamily *fam = pglist_fonts();

    while (fam->name && stricmp(fam->name, family))
        fam++;
    if (!fam->name)
        return 0;

    // Default to Normal weight.
    if (weight == 0)
        weight = 400;

    // Score each option and try to find the best.
    Pgface  *best = fam->faces;
    int     bestscore = -1000;

    for (Pgface *fac = fam->faces; fac->family; fac++) {
        int score = 0 +
            -abs((int) 5 - (int) fac->width)             * 10
            -abs((int) weight - (int) fac->weight) / 100 * 3
            -abs((int) sloped - (int) fac->sloped)       * 2
            + 0;

        if (score > bestscore) {
            best = fac;
            bestscore = score;
        }
    }

    return pgopen_font_file(best->path, best->index);
}
Pgfont *pgopen_font_file(const char *path, unsigned index) {
    if (!path)
        return 0;

    size_t  size = 0;
    void    *data = _pgmap_file(path, &size);

    Pgfont *font = pgopen_font(data, size, index);
    if (!font) {
        _pgunmap_file(data, size);
        return 0;
    }
    return font;
}
Pgfont *pgopen_font(const uint8_t *data, size_t size, unsigned index) {
    if (data) {
        Pgfont *font = 0;
        return (font = _pgopen_opentype_font(data, size, index))
            ? font: 0;
    }
    return 0;
}
void pgfree_font(Pgfont *font) {
    if (font) {
        if (font->v && font->v->free)
            font->v->free(font);
        _pgunmap_file((void*) font->data, font->size);
        free(font->cmap);
        free(font);
    }
}
float pgfontpropf(Pgfont *font, Pgfontprop id) {
    if (font && font->v && font->v->propf)
        return font->v->propf(font, id);
    return 0.0f;
}
int pgfontpropi(Pgfont *font, Pgfontprop id) {
    return (int) pgfontpropf(font, id);
}
const char *pgfontprops(Pgfont *font, Pgfontprop id) {
    if (font && font->v && font->v->props)
        return font->v->props(font, id);
    return "";
}
Pgfont *pgscale_font(Pgfont *font, float sx, float sy) {
    if (sx == 0.0f) sx = sy;
    if (sy == 0.0f) sy = sx;
    if (font && font->units != 0.0f && sx != 0.0f && sy != 0.0f) {
        font->sx = sx / font->units;
        font->sy = sy / font->units;
    }
    return font;
}
unsigned pgget_glyph(Pgfont *font, unsigned codepoint) {
    return font->cmap[codepoint < 65536? codepoint: 0xfffd];
}
Pgpt pgdraw_glyph(Pg *g, Pgfont *font, Pgpt p, unsigned glyph) {
    if (g && font && glyph < font->nglyphs && font->v && font->v->glyph)
        font->v->glyph(g, font, p, glyph);
    return font? pgpt(p.x + pgmeasure_glyph(font, glyph), p.y): p;
}
Pgpt pgdraw_char(Pg *g, Pgfont *font, Pgpt p, unsigned codepoint) {
    return pgdraw_glyph(g, font, p, pgget_glyph(font, codepoint));
}
Pgpt pgdraw_chars(Pg *g, Pgfont *font, Pgpt p, const char *s, unsigned n) {
    if (g && font && s) {
        const uint8_t *end = (uint8_t*) s + n;
        for (const uint8_t *i = (uint8_t*) s; i < end; )
            p = pgdraw_char(g, font, p, pgread_utf8(&i));
    }
    return p;
}
Pgpt pgdraw_string(Pg *g, Pgfont *font, Pgpt p, const char *str) {
    return pgdraw_chars(g, font, p, str, str? (unsigned) strlen(str): 0);
}
Pgpt pgvprintf(Pg *g, Pgfont *font, Pgpt p, const char *str, va_list ap) {
    va_list tmp;
    va_copy(tmp, ap);
    char    small[1024];
    size_t  n = (size_t) vsnprintf(small, sizeof small, str, tmp);
    char    *buf = n + 1 <= sizeof small? small: malloc(n + 1);
    if (buf != small)
        snprintf(buf, n + 1, str, ap);

    float       left = p.x;
    float       lh = pgfontpropf(font, PG_FONT_EM);
    char        *s = buf;
    for (char *brk; (brk = strchr(s, '\n')); s = brk + 1) {
        pgdraw_chars(g, font, p, s, (unsigned) (brk - s));
        p = pgpt(left, p.y + lh);
    }
    p = pgdraw_chars(g, font, p, s, (unsigned) (n - (size_t) (s - buf)));

    if (buf != small)
        free(buf);
    va_end(tmp);
    return p;
}
Pgpt pgprintf(Pg *g, Pgfont *font, Pgpt p, const char *str, ...) {
    va_list ap;
    va_start(ap, str);
    Pgpt result = pgvprintf(g, font, p, str, ap);
    va_end(ap);
    return result;
}
float pgmeasure_glyph(Pgfont *font, unsigned glyph) {
    if (font && glyph < font->nglyphs && font->v && font->v->measureglyph)
        return font->v->measureglyph(font, glyph);
    return 0.0f;
}
float pgmeasure_char(Pgfont *font, unsigned codepoint) {
    return pgmeasure_glyph(font, pgget_glyph(font, codepoint));
}
float pgmeasure_chars(Pgfont *font, const char *s, unsigned n) {
    if (font && s && n) {
        float   x = 0.0f;
        for (unsigned i = 0; i < n; i++)
            x += pgmeasure_char(font, (unsigned char) s[i]);
        return x;
    }
    return 0.0f;
}
float pgmeasure_string(Pgfont *font, const char *str) {
    return pgmeasure_chars(font, str, str? (unsigned) strlen(str): 0);
}
