#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wctype.h>
#include <pg.h>
#include "pg.internal.h"


typedef struct Pgstate Pgstate;


struct Pgstate {
    Pgmat       ctm;
    Pgpaint     fill;
    Pgpaint     stroke;
    float       line_width;
    Pgline_cap  line_cap;
    float       flatness;
    Pgfill_rule rule;
    Pgrect      clip;
    Pgtext_pos  text_pos;
    bool        underline;
};

struct Pg {
    const Pgcanvas_methods *v;
    void        *sys;           // Implementation data.

    float       width;
    float       height;
    Pgpath      path;
    Pgstate     s;
    Pgstate     saved[16];
    unsigned    nsaved;
};

struct Pgfont {
    const Pgfont_methods *v;
    void            *sys;           // Implementation data.
    float           units;          // Units per EM.
    float           sx;             // Scale X.
    float           sy;             // Scale Y.
    unsigned        nglyphs;        // Number of glyphs in font.
    const uint8_t   *data;          // Pointer to file content.
    size_t          size;           // Size of file.
    unsigned        index;          // Index within file.
    uint16_t        *cmap;          // Character-to-glyph mapping.
};



static Pgfamily     *allfamilies;




Pg *pg_new_canvas(const Pgcanvas_methods *v, unsigned width, unsigned height) {
    Pg *g = new(Pg,
        .v = v,
        .width = (float) width,
        .height = (float) height,
        .path = {0, 0, {FLT_MAX, FLT_MAX}, {FLT_MIN, FLT_MIN}},
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
            .line_width = 1.0f,
            .line_cap = PG_BUTT_CAP,
            .flatness = 1.0f,
            .rule = PG_NONZERO_RULE,
            .clip = {{0.0f, 0.0f}, {(float) width, (float) height}},
        });

    if (!g->v->init || !(g->sys = g->v->init(g))) {
        free(g);
        return 0;
    }
    return g;
}

void *pg_get_canvas_impl(const Pg *g) {
    return g->sys;
}

void pg_free_canvas(Pg *g) {
    if (g) {
        if (g->v && g->v->free)
            g->v->free(g);
        free(g);
    }
}

Pg *pg_resize_canvas(Pg *g, unsigned width, unsigned height) {
    if (g && width > 0 && height > 0) {
        if (g->v && g->v->resize)
            g->v->resize(g, width, height);
        g->width = (float) width;
        g->height = (float) height;
        g->s.clip = pg_rect(0.0f, 0.0f, (float) width, (float) height);
    }
    return g;
}

Pgpt pg_get_size(Pg *g) {
    return g? pg_pt(g->width, g->height): pg_zero_pt();
}

Pgpath pg_get_path(Pg *g) {
    return g? g->path: (Pgpath) {0, 0, {FLT_MAX, FLT_MAX}, {FLT_MIN, FLT_MIN}};
}

Pgrect pg_get_bbox(Pg *g) {
    return g? pg_rect_abs_pt(g->path.min, g->path.max):
        pg_rect(FLT_MAX, FLT_MAX, FLT_MIN, FLT_MIN);
}

Pgpaint pg_solid_color(Pgcolorspace cspace, Pgcolor colour) {
    return (Pgpaint) {
        .type = PG_SOLID_PAINT,
        .cspace = cspace,
        .colours[0] = colour,
        .nstops = 1,
    };
}

Pgpaint pg_solid(Pgcolorspace cspace, float x, float y, float z, float a) {
    return pg_solid_color(cspace, (Pgcolor) {x, y, z, a});
}

Pgpaint pg_linear_pt(Pgcolorspace cspace, Pgpt a, Pgpt b) {
    return (Pgpaint) {
        .type = PG_LINEAR_PAINT,
        .cspace = cspace,
        .colours[0] = (Pgcolor) {0.0f, 0.0f, 0.0f, 1.0f},
        .stops[0] = 0.0f,
        .nstops = 0,
        .a = a,
        .b = b,
    };
}

Pgpaint pg_linear_rect(Pgcolorspace cspace, Pgrect r) {
    return pg_linear_pt(cspace, r.p, pg_add_pts(r.p, r.size));
}

Pgpaint pg_linear(Pgcolorspace cspace, float ax, float ay, float bx, float by) {
    return pg_linear_pt(cspace, pg_pt(ax, ay), pg_pt(bx, by));
}

Pgpaint *pg_set_linear_pt(Pgpaint *paint, Pgpt a, Pgpt b) {
    if (paint && paint->type == PG_LINEAR_PAINT) {
        paint->a = a;
        paint->b = b;
    }
    return paint;
}

Pgpaint *pg_set_linear_rect(Pgpaint *paint, Pgrect r) {
    return pg_set_linear_pt(paint, r.p, pg_rect_end(r));
}

Pgpaint *pg_set_linear(Pgpaint *paint, float ax, float ay, float bx, float by) {
    return pg_set_linear_rect(paint, pg_rect(ax, ay, bx, by));
}


Pgpaint *pg_add_stop_color(Pgpaint *paint, float t, Pgcolor colour) {
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

Pgpaint *pg_add_stop(Pgpaint *paint, float t, float x, float y, float z, float a) {
    return pg_add_stop_color(paint, t, (Pgcolor) {x, y, z, a});
}

Pgpaint *pg_set_colorspace(Pgpaint *paint, Pgcolorspace cspace) {
    if (paint)
        paint->cspace = cspace;
    return paint;
}

Pg *pg_save(Pg *g) {
    if (g && g->nsaved + 1 < sizeof g->saved / sizeof *g->saved)
        g->saved[g->nsaved++] = g->s;
    return g;
}

Pg *pg_restore(Pg *g) {
    if (g && g->nsaved)
        g->s = g->saved[--g->nsaved];
    return g;
}

Pgrect pg_get_clip(const Pg *g) {
    return g? g->s.clip: pg_empty_rect();
}

Pgmat pg_get_ctm(const Pg *g) {
    return g? g->s.ctm: pg_ident_mat();
}

Pgrect pg_set_clip(Pg *g, float x, float y, float sx, float sy) {
    return pg_set_clip_rect(g, pg_rect(x, y, sx, sy));
}

Pgrect pg_set_clip_abs(Pg *g, float ax, float ay, float bx, float by) {
    return pg_set_clip_rect(g, pg_rect_abs(ax, ay, bx, by));
}

Pgrect pg_set_clip_rect(Pg *g, Pgrect clip) {
    if (g) {
        Pgrect old = g->s.clip;
        g->s.clip = clip;
        return old;
    }
    return pg_empty_rect();
}

Pgmat pg_set_ctm(Pg *g, Pgmat ctm) {
    if (g) {
        Pgmat old = g->s.ctm;
        g->s.ctm = ctm;
        return old;
    }
    return pg_ident_mat();
}

Pgpaint pg_get_fill(const Pg *g) {
    return g? g->s.fill: pg_solid(PG_SRGB, 0.0f, 0.0f, 0.0f, 1.0f);
}

Pgtext_pos pg_get_text_pos(const Pg *g) {
    return g? g->s.text_pos: PG_TEXT_POS_TOP;
}

bool pg_get_underline(const Pg *g) {
    return g? g->s.underline: false;
}

Pgpaint pg_set_fill(Pg *g, Pgpaint paint) {
    if (g) {
        Pgpaint old = g->s.fill;
        g->s.fill = paint;
        return old;
    }
    return pg_solid(PG_SRGB, 0.0f, 0.0f, 0.0f, 1.0f);
}

Pgpaint pg_get_stroke(const Pg *g) {
    return g? g->s.stroke: (Pgpaint) {0};
}

Pgpaint pg_set_stroke(Pg *g, Pgpaint paint) {
    if (g) {
        Pgpaint old = g->s.stroke;
        g->s.stroke = paint;
        return old;
    }
    return pg_solid(PG_SRGB, 0.0f, 0.0f, 0.0f, 1.0f);
}

float pg_get_line_width(const Pg *g) {
    return g? g->s.line_width: 1.0f;
}

float pg_set_line_width(Pg *g, float line_width) {
    if (g) {
        float old = g->s.line_width;
        if (line_width > 0.0f)
            g->s.line_width = line_width;
        return old;
    }
    return 1.0f;
}

Pgline_cap pg_get_line_cap(const Pg *g) {
    return g? g->s.line_cap: PG_BUTT_CAP;
}

Pgline_cap pg_set_line_cap(Pg *g, Pgline_cap line_cap) {
    if (g) {
        Pgline_cap old = g->s.line_cap;
        if (line_cap == PG_BUTT_CAP || line_cap == PG_SQUARE_CAP)
            g->s.line_cap = line_cap;
        return old;
    }
    return PG_BUTT_CAP;
}

float pg_get_flatness(const Pg *g) {
    return g? g->s.flatness: 1.0f;
}

float pg_set_flatness(Pg *g, float flatness) {
    if (g) {
        float old = g->s.flatness;
        if (flatness >= 0.0f && flatness <= 100.0f)
            g->s.flatness = flatness;
        return old;
    }
    return 1.0f;
}

Pgfill_rule pg_get_fill_rule(const Pg *g) {
    return g? g->s.rule: PG_NONZERO_RULE;
}

Pgfill_rule pg_set_fill_rule(Pg *g, Pgfill_rule rule) {
    if (g) {
        Pgfill_rule old = g->s.rule;
        if (rule == PG_NONZERO_RULE || rule == PG_EVEN_ODD_RULE)
            g->s.rule = rule;
        return old;
    }
    return PG_NONZERO_RULE;
}

Pgtext_pos pg_set_text_pos(Pg *g, Pgtext_pos pos) {
    if (g) {
        Pgtext_pos old = g->s.text_pos;

        switch (pos) {
        case PG_TEXT_POS_TOP:
        case PG_TEXT_POS_BOTTOM:
        case PG_TEXT_POS_BASELINE:
        case PG_TEXT_POS_CENTER:
            g->s.text_pos = pos;
        }

        return old;
    }
    return PG_TEXT_POS_TOP;
}

bool pg_set_underline(Pg *g, bool underline) {
    if (g) {
        bool old = g->s.underline;
        g->s.underline = underline;
        return old;
    }
    return false;
}

static size_t path_capacity(size_t n) {
    return (n + 31UL) & ~31UL;
}

static Pgpart_form prev_part(Pgpath path) {
    return path.nparts? path.parts[path.nparts - 1].form: PG_PART_CLOSE;
}

static unsigned part_count(Pgpart_form form) {
    switch (form) {
    case PG_PART_MOVE:      return 1;
    case PG_PART_LINE:      return 1;
    case PG_PART_CURVE3:    return 2;
    case PG_PART_CURVE4:    return 3;
    case PG_PART_CLOSE:     return 0;
    }
    return 0;
}

static void add_pt(Pgpath *path, Pgpart part) {
    if (path->nparts + 1 >= path_capacity(path->nparts)) {
        size_t cap = path_capacity(path->nparts + 1);
        path->parts = realloc(path->parts, cap * sizeof *path->parts);
    }

    path->parts[path->nparts++] = part;

    for (unsigned i = 0; i < part_count(part.form); i++) {
        path->min.x = fminf(path->min.x, part.pt[i].x);
        path->min.y = fminf(path->min.y, part.pt[i].y);
        path->max.x = fmaxf(path->max.x, part.pt[i].x);
        path->max.y = fmaxf(path->max.y, part.pt[i].y);
    }
}

Pgpt pg_cur(Pg *g) {
    if (g && g->path.nparts) {
        Pgpart *p = &g->path.parts[g->path.nparts - 1];
        switch (p->form) {
        case PG_PART_CLOSE: return p->pt[0];
        case PG_PART_MOVE:  return p->pt[0];
        case PG_PART_LINE:  return p->pt[0];
        case PG_PART_CURVE3:return p->pt[1];
        case PG_PART_CURVE4:return p->pt[2];
        }
    }
    return pg_pt(0.0f, 0.0f);
}

Pg *pg_move_to(Pg *g, float x, float y) {
    return pg_move_to_pt(g, pg_pt(x, y));
}

Pg *pg_move_to_pt(Pg *g, Pgpt p) {
    if (g)
        add_pt(&g->path, (Pgpart) {PG_PART_MOVE, {p}});
    return g;
}

Pg *pg_rel_move_to(Pg *g, float x, float y) {
    return pg_rel_move_to_pt(g, pg_pt(x, y));
}

Pg *pg_rel_move_to_pt(Pg *g, Pgpt p) {
    return pg_move_to_pt(g, pg_add_pts(pg_cur(g), p));
}

Pg *pg_line_to(Pg *g, float x, float y) {
    return pg_line_to_pt(g, pg_pt(x, y));
}

Pg *pg_line_to_pt(Pg *g, Pgpt p) {
    if (g && prev_part(g->path) != PG_PART_CLOSE)
        add_pt(&g->path, (Pgpart) {PG_PART_LINE, {p}});
    return g;
}

Pg *pg_rel_line_to(Pg *g, float x, float y) {
    return pg_rel_line_to_pt(g, pg_pt(x, y));
}

Pg *pg_rel_line_to_pt(Pg *g, Pgpt p) {
    return pg_line_to_pt(g, pg_add_pts(pg_cur(g), p));
}

Pg *pg_curve3_to(Pg *g, float bx, float by, float cx, float cy) {
    return pg_curve3_to_pt(g, pg_pt(bx, by), pg_pt(cx, cy));
}

Pg *pg_curve3_to_pt(Pg *g, Pgpt b, Pgpt c) {
    if (g && prev_part(g->path) != PG_PART_CLOSE)
        add_pt(&g->path, (Pgpart) {PG_PART_CURVE3, {b, c}});
    return g;
}

Pg *pg_rel_curve3_to(Pg *g, float bx, float by, float cx, float cy) {
    return pg_rel_curve3_to_pt(g, pg_pt(bx, by), pg_pt(cx, cy));
}

Pg *pg_rel_curve3_to_pt(Pg *g, Pgpt b, Pgpt c) {
    b = pg_add_pts(pg_cur(g), b);
    c = pg_add_pts(b, c);
    return pg_curve3_to_pt(g, b, c);
}

Pg *pg_curve4_to(Pg *g, float bx, float by, float cx, float cy, float dx, float dy) {
    return pg_curve4_to_pt(g, pg_pt(bx, by), pg_pt(cx, cy), pg_pt(dx, dy));
}

Pg *pg_curve4_to_pt(Pg *g, Pgpt b, Pgpt c, Pgpt d) {
    if (g && prev_part(g->path) != PG_PART_CLOSE)
        add_pt(&g->path, (Pgpart) {PG_PART_CURVE4, {b, c, d}});
    return g;
}

Pg *pg_rel_curve4_to(Pg *g, float bx, float by, float cx, float cy, float dx, float dy) {
    return pg_rel_curve4_to_pt(g, pg_pt(bx, by), pg_pt(cx, cy), pg_pt(dx, dy));
}

Pg *pg_rel_curve4_to_pt(Pg *g, Pgpt b, Pgpt c, Pgpt d) {
    b = pg_add_pts(pg_cur(g), b);
    c = pg_add_pts(b, c);
    d = pg_add_pts(c, d);
    return pg_curve4_to_pt(g, b, c, d);
}

Pg *pg_close_path(Pg *g) {
    if (g && prev_part(g->path) != PG_PART_CLOSE)
        add_pt(&g->path, (Pgpart) {PG_PART_CLOSE, {pg_cur(g)}});
    return g;
}

Pg *pg_clear_path(Pg *g) {
    if (g) {
        g->path.min = pg_pt(FLT_MAX, FLT_MAX);
        g->path.max = pg_pt(FLT_MIN, FLT_MIN);
        g->path.nparts = 0;
    }
    return g;
}

Pg *pg_rect_path(Pg *g, float x, float y, float sx, float sy) {
    if (g) {
        pg_move_to(g, x, y);
        pg_rel_line_to(g, sx, 0.0f);
        pg_rel_line_to(g, 0.0f, sy);
        pg_rel_line_to(g, -sx, 0.0f);
        pg_close_path(g);
    }
    return g;
}

Pg *pg_rect_path_abs(Pg *g, float ax, float ay, float bx, float by) {
    return pg_rect_path(g, ax, ay, bx - ax, by - ay);
}

Pg *pg_rect_path_pt(Pg *g, Pgrect r) {
    return pg_rect_path(g, r.p.x, r.p.y, r.size.x, r.size.y);
}

Pg *pg_rrect_path(Pg *g, float x, float y, float sx, float sy, float rad) {
    if (g) {
        if (rad == 0.0f)
            pg_rect_path(g, x, y, sx, sy);

        else if (rad > 0.0f) {
            float   rx = fminf(sx, rad);
            float   ry = fminf(sy, rad);

            pg_move_to(g,       x + sx - rx,    y);
            pg_rel_curve3_to(g, rx,             0.0f,       0.0f,   ry);
            pg_rel_line_to(g,   0.0f,           sy - ry - ry);
            pg_rel_curve3_to(g, 0.0f,           ry,         -rx,    0.0f);
            pg_rel_line_to(g,   -(sx - rx - rx), 0.0f);
            pg_rel_curve3_to(g, -rx,            0.0f,       0.0f,   -ry);
            pg_rel_line_to(g,   0.0f,           -(sy - ry - ry));
            pg_rel_curve3_to(g, 0.0f,           -ry,        rx,     0.0f);
            pg_close_path(g);
        }
    }
    return g;
}

Pg *pg_rrect_path_abs(Pg *g, float ax, float ay, float bx, float by, float rad) {
    return pg_rrect_path(g, ax, ay, bx - ax, by - ay, rad);
}

Pg *pg_rrect_path_pt(Pg *g, Pgrect r, float rad) {
    return pg_rrect_path(g, r.p.x, r.p.y, r.size.x, r.size.y, rad);
}

Pg *pg_ctm_identity(Pg *g) {
    if (g)
        g->s.ctm = pg_ident_mat();
    return g;
}

Pg *pg_ctm_translate(Pg *g, float x, float y) {
    if (g)
        g->s.ctm = pg_translate_mat(g->s.ctm, x, y);
    return g;
}

Pg *pg_ctm_translate_pt(Pg *g, Pgpt v) {
    if (g)
        g->s.ctm = pg_translate_mat(g->s.ctm, v.x, v.y);
    return g;
}

Pg *pg_ctm_scale(Pg *g, float x, float y) {
    if (g)
        g->s.ctm = pg_scale_mat(g->s.ctm, x, y);
    return g;
}

Pg *pg_ctm_scale_pt(Pg *g, Pgpt v) {
    if (g)
        g->s.ctm = pg_scale_mat(g->s.ctm, v.x, v.y);
    return g;
}

Pg *pg_ctm_rotate(Pg *g, float rads) {
    if (g)
        g->s.ctm = pg_rotate_mat(g->s.ctm, rads);
    return g;
}

Pg *pg_clear(Pg *g, Pgpaint paint) {
    if (g && g->v && g->v->clear)
        g->v->clear(g, paint);
    return g;
}

Pg *pg_fill(Pg *g) {
    if (g) {
        if (g->v && g->v->fill)
            g->v->fill(g);
        pg_clear_path(g);
    }
    return g;
}

Pg *pg_stroke(Pg *g) {
    if (g) {
        if (g->v && g->v->stroke)
            g->v->stroke(g);
        pg_clear_path(g);
    }
    return g;
}

Pg *pg_fill_stroke(Pg *g) {
    if (g) {
        if (g->v && g->v->fill_stroke)
            g->v->fill_stroke(g);
        pg_clear_path(g);
    }
    return g;
}

void *pg_get_font_impl(const Pgfont *font) {
    return font? font->sys: 0;
}

static int compare_face(const void *ap, const void *bp) {
    const Pgface  *a = (const Pgface*) ap;
    const Pgface  *b = (const Pgface*) bp;
    int r;
    return (r = stricmp(a->family, b->family))? r:
           a->width != b->width?    (int) a->width - (int) b->width:
           a->weight != b->weight?  (int) a->weight - (int) b->weight:
           a->is_italic != b->is_italic?  (int) a->is_italic - (int) b->is_italic:
           (r = stricmp(a->style, b->style))? r:
           0;
}

Pgfamily *pg_list_fonts() {

    if (allfamilies)
        return allfamilies;

    char        **files = _pgget_font_files();

    if (!files)
        return calloc(1, sizeof(Pgfamily));

    // Get font properties.
    Pgface      *faces = 0;
    unsigned    nfaces = 0;
    Pgfont      *font;

    for (unsigned i = 0; files[i]; i++) {
        if ((font = pg_open_font_file(files[i], 0))) {
            Pgface  f = {0};
            f.family = strdup(pg_font_prop_string(font, PG_FONT_FAMILY));
            f.style = strdup(pg_font_prop_string(font, PG_FONT_STYLE));
            f.path = files[i];
            f.index = (unsigned) pg_font_prop_int(font, PG_FONT_INDEX);
            f.width = (unsigned) pg_font_prop_int(font, PG_FONT_WIDTH_CLASS);
            f.weight = (unsigned) pg_font_prop_int(font, PG_FONT_WEIGHT);
            f.is_fixed = (unsigned) pg_font_prop_int(font, PG_FONT_IS_FIXED);
            f.is_italic = pg_font_prop_int(font, PG_FONT_IS_ITALIC);
            f.is_serif = (unsigned) pg_font_prop_int(font, PG_FONT_IS_SERIF);
            f.is_sans_serif = (unsigned) pg_font_prop_int(font, PG_FONT_IS_SANS_SERIF);
            f.style_class = (uint8_t) pg_font_prop_int(font, PG_FONT_STYLE_CLASS);
            f.style_subclass = (uint8_t) pg_font_prop_int(font, PG_FONT_STYLE_SUBCLASS);

            for (unsigned i = 0; i < 10; i++)
                f.panose[i] = (uint8_t) pg_font_prop_int(font, PG_FONT_PANOSE_1 + i);

            pg_free_font(font);

            faces = realloc(faces, (nfaces + 2) * sizeof *faces);
            faces[nfaces++] = f;
        }
    }

    if (!faces)
        faces = malloc(sizeof *faces);

    faces[nfaces] = (Pgface) { 0 };
    free(files);

    // Sort them and put them into families.
    qsort(faces, nfaces, sizeof *faces, compare_face);

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

    if (!families)
        families = malloc(sizeof *families);

    families[nfamilies] = (Pgfamily) { 0 };
    free(faces);

    allfamilies = families;
    return families;
}

Pgfont *pg_find_font(const char *family, unsigned weight, bool italic) {
    if (!family)
        return 0;

    Pgfamily *fam = pg_list_fonts();

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
        int score =
            -abs((int) 5 - (int) fac->width)             * 10
            -abs((int) weight - (int) fac->weight) / 100 * 3
            -abs((int) italic - (int) fac->is_italic)    * 2
            + 0;

        if (score > bestscore) {
            best = fac;
            bestscore = score;
        }
    }

    return pg_open_font_file(best->path, best->index);
}

Pgfont *pg_open_font(const uint8_t *data, size_t size, unsigned index) {
    if (data && size) {
        Pgfont *font;

        if ((font = pg_open_otf_font(data, size, index)))
            return font;
        return 0;
    }
    return 0;
}

Pgfont *pg_open_font_file(const char *path, unsigned index) {
    if (!path)
        return 0;

    size_t  size = 0;
    void    *data = _pgmap_file(path, &size);

    Pgfont *font = pg_open_font(data, size, index);
    if (!font) {
        _pgunmap_file(data, size);
        return 0;
    }
    return font;
}


Pgfont *pg_new_font(const Pgfont_methods *v, const uint8_t *data, size_t size, unsigned index) {
    if (data) {
        Pgfont *font = new(Pgfont,
            .v = v,
            .sx = 1.0f,
            .sy = 1.0f,
            .data = data,
            .size = size,
            .index = index,
            .cmap =  malloc(sizeof(uint16_t) * 65536));

        if (font->v->init && (font->sys = font->v->init(font, data, size, index)))
            return font;
        free(font);
    }
    return 0;
}

Pgfont *pg_init_font(Pgfont *font,
                    float units,
                    unsigned nglyphs,
                    const uint16_t cmap[65536])
{
    if (font) {
        font->units = units;
        font->nglyphs = nglyphs;
        memcpy(font->cmap, cmap, sizeof *font->cmap * 65536);
    }
    return font;
}

void pg_free_font(Pgfont *font) {
    if (font) {
        if (font->v && font->v->free)
            font->v->free(font);
        _pgunmap_file((void*) font->data, font->size);
        free(font->cmap);
        free(font);
    }
}

Pgpt pg_get_font_scale(Pgfont *font) {
    return font? pg_pt(font->sx, font->sy): pg_zero_pt();
}

unsigned pg_get_nglyphs(Pgfont *font) {
    return font? font->nglyphs: 0;
}

float pg_get_em_units(Pgfont *font) {
    return font? font->units: 0;
}

float pg_font_prop_float(Pgfont *font, Pgfont_prop id) {
    if (font && font->v && font->v->propf)
        return font->v->propf(font, id);
    return 0.0f;
}

int pg_font_prop_int(Pgfont *font, Pgfont_prop id) {
    return (int) pg_font_prop_float(font, id);
}

const char *pg_font_prop_string(Pgfont *font, Pgfont_prop id) {
    if (font && font->v && font->v->props)
        return font->v->props(font, id);
    return "";
}

Pgfont *pg_scale_font(Pgfont *font, float sx, float sy) {
    if (sx == 0.0f) sx = sy;
    if (sy == 0.0f) sy = sx;
    if (font && font->units != 0.0f && sx != 0.0f && sy != 0.0f) {
        font->sx = sx / font->units;
        font->sy = sy / font->units;
    }
    return font;
}

float pg_get_font_height(Pgfont *font) {
    return pg_font_prop_float(font, PG_FONT_EM);
}

unsigned pg_get_glyph(Pgfont *font, unsigned codepoint) {
    return font->cmap[codepoint < 65536? codepoint: 0xfffd];
}

Pgpt pg_glyph_path(Pg *g, Pgfont *font, Pgpt p, unsigned glyph) {
    if (g && font && glyph < font->nglyphs && font->v && font->v->glyph_path)
        font->v->glyph_path(g, font, p, glyph);
    return font? pg_pt(p.x + pg_measure_glyph(font, glyph).x, p.y): p;
}

Pgpt pg_char_path(Pg *g, Pgfont *font, Pgpt p, unsigned codepoint) {
    return pg_glyph_path(g, font, p, pg_get_glyph(font, codepoint));
}

Pgpt pg_chars_path(Pg *g, Pgfont *font, Pgpt p, const char *s, unsigned n) {
    if (g && font && s) {
        const uint8_t *end = (uint8_t*) s + n;
        for (const uint8_t *i = (uint8_t*) s; i < end; )
            p = pg_char_path(g, font, p, pg_read_utf8(&i));
    }
    return p;
}

Pgpt pg_string_path(Pg *g, Pgfont *font, Pgpt p, const char *str) {
    return pg_chars_path(g, font, p, str, str? (unsigned) strlen(str): 0);
}

Pgpt pg_vprintf(Pg *g, Pgfont *font, Pgpt p, const char *str, va_list ap) {
    va_list tmp;
    va_copy(tmp, ap);
    char    small[1024];
    size_t  n = (size_t) vsnprintf(small, sizeof small, str, tmp);
    char    *buf = n + 1 <= sizeof small? small: malloc(n + 1);
    if (buf != small)
        snprintf(buf, n + 1, str, ap);

    float       left = p.x;
    float       lh = pg_font_prop_float(font, PG_FONT_EM);
    char        *s = buf;
    for (char *brk; (brk = strchr(s, '\n')); s = brk + 1) {
        pg_chars_path(g, font, p, s, (unsigned) (brk - s));
        p = pg_pt(left, p.y + lh);
    }
    p = pg_chars_path(g, font, p, s, (unsigned) (n - (size_t) (s - buf)));

    if (buf != small)
        free(buf);
    va_end(tmp);
    return p;
}

Pgpt pg_printf(Pg *g, Pgfont *font, Pgpt p, const char *str, ...) {
    va_list ap;
    va_start(ap, str);
    Pgpt result = pg_vprintf(g, font, p, str, ap);
    va_end(ap);
    return result;
}

Pgpt pg_measure_glyph(Pgfont *font, unsigned glyph) {
    if (font && glyph < font->nglyphs && font->v && font->v->measure_glyph)
        return font->v->measure_glyph(font, glyph);
    return pg_pt(0.0f, 0.0f);
}

Pgpt pg_measure_char(Pgfont *font, unsigned codepoint) {
    return pg_measure_glyph(font, pg_get_glyph(font, codepoint));
}

Pgpt pg_measure_chars(Pgfont *font, const char *s, unsigned n) {
    if (font && s && n) {
        Pgpt    p = pg_pt(0.0f, 0.0f);
        for (unsigned i = 0; i < n; i++) {
            Pgpt q = pg_measure_char(font, (unsigned char) s[i]);
            p = pg_pt(p.x + q.x, fmaxf(p.y, q.y));
        }
        return p;
    }
    return pg_pt(0.0f, 0.0f);
}

Pgpt pg_measure_string(Pgfont *font, const char *str) {
    return pg_measure_chars(font, str, str? (unsigned) strlen(str): 0);
}

unsigned pg_fit_chars(Pgfont *font, const char *s, unsigned n, float width) {
    if (font && s) {
        float           total = 0.0f;
        unsigned        nfit = 0;
        const uint8_t   *i = (uint8_t*) s;
        const uint8_t   *end = i + n;

        while (i < end) {
            float size = pg_measure_char(font, pg_read_utf8(&i)).x;

            if (total + size > width)
                break;
            total += size;
            nfit++;
        }
        return nfit;
    }
    return 0;
}

unsigned pg_fit_string(Pgfont *font, const char *str, float width) {
    return pg_fit_chars(font, str, str? (unsigned) strlen(str): 0, width);
}

Pgcolor pg_lch_to_lab(Pgcolor lch) {
    float c = lch.y;
    float h = lch.z * 2.0f * 3.14159f;
    return (Pgcolor) {lch.x, c * cosf(h), c * sinf(h), lch.a};
}

Pgcolor pg_lab_to_xyz(Pgcolor lab) {
    float l = lab.x;
    float y = (l + 16.0f) / 116.0f;
    float x = y + lab.y / 500.0f;
    float z = y - lab.z / 200.0f;
    x = x > 24.0f / 116.0f? x*x*x: (x - 16.0f/116.0f) * 108.0f/841.0f;
    y = y > 24.0f / 116.0f? y*y*y: (y - 16.0f/116.0f) * 108.0f/841.0f;
    z = z > 24.0f / 116.0f? z*z*z: (z - 16.0f/116.0f) * 108.0f/841.0f;
    return (Pgcolor) {x * 950.4f, y * 1000.0f, z * 1088.8f, lab.a};
}

Pgcolor pg_xyz_to_rgb(Pgcolor xyz) {
    float x = xyz.x;
    float y = xyz.y;
    float z = xyz.z;
    float r = x * +3.2404542f + y * -1.5371385f + z * -0.4985314f;
    float g = x * -0.9692660f + y * +1.8760108f + z * +0.0415560f;
    float b = x * +0.0556434f + y * -0.2040259f + z * +1.0572252f;
    return (Pgcolor) {r, g, b, xyz.a};
}

Pgcolor pg_gamma(Pgcolor rgb) {
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;
    return (Pgcolor) {
        r < 0.0031308f? 12.92f * r: 1.055f * powf(r, 1.0f / 2.20f) - 0.055f,
        g < 0.0031308f? 12.92f * g: 1.055f * powf(g, 1.0f / 2.20f) - 0.055f,
        b < 0.0031308f? 12.92f * b: 1.055f * powf(b, 1.0f / 2.20f) - 0.055f,
        rgb.a
    };
}

Pgcolor pg_convert_color(Pgcolorspace cspace, Pgcolor colour) {
    return  cspace == 0? pg_gamma(colour):
            cspace == 1? pg_gamma(pg_xyz_to_rgb(pg_lab_to_xyz(pg_lch_to_lab(colour)))):
            cspace == 2? pg_gamma(pg_xyz_to_rgb(pg_lab_to_xyz(colour))):
            cspace == 3? pg_gamma(pg_xyz_to_rgb(colour)):
            colour;
}
