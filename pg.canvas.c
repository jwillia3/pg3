#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <pg3.h>
#include <internal.h>

static const PgPaint black_paint = {
    .type = PG_SOLID_PAINT,
    .cspace = PG_LINEAR_RGB,
    .colors[0] = { 0.0f, 0.0f, 0.0f, 1.0f },
    .nstops = 1,
};

static const PgPaint white_paint = {
    .type = PG_SOLID_PAINT,
    .cspace = PG_LINEAR_RGB,
    .colors[0] = { 1.0f, 1.0f, 1.0f, 1.0f },
    .nstops = 1,
};


PgPt
pg_size(Pg *g)
{
    if (!g)
        return zero();
    return PgPt(g->sx, g->sy);
}



Pg
pg_init_canvas(const PgCanvasImpl *v, float width, float height)
{
    Pg canvas = {
                    .v = v,
                    .sx = width,
                    .sy = height,
                    .path = pg_path(),
                    .nsaved = 0,
                };

    pg_reset_state(&canvas);

    return canvas;
}


void
pg_reset_state(Pg *g)
{
    if (!g)
        return;

    g->s = (PgState) {
        .ctm = pg_ident_tm(),
        .clear = &white_paint,
        .fill = &black_paint,
        .stroke = &black_paint,
        .line_width = 1.0f,
        .line_cap = PG_BUTT_CAP,
        .flatness = 1.0f,
        .fill_rule = PG_NONZERO_RULE,
        .gamma = 1.8f,
        .clip_x = 0.0f,
        .clip_y = 0.0f,
        .clip_sx = g->sx,
        .clip_sy = g->sy,
        .text_pos = PG_TEXT_POS_TOP,
        .underline = false,
    };
}

PgPt
pg_apply_tm(PgTM ctm, PgPt p)
{
    return PgPt(ctm.a * p.x + ctm.c * p.y + ctm.e,
                ctm.b * p.x + ctm.d * p.y + ctm.f);
}


PgTM
pg_mul_tm(PgTM x, PgTM y)
{
    return (PgTM) {
        (x.a * y.a) + (x.b * y.c) + (0.0f * y.e),
        (x.a * y.b) + (x.b * y.d) + (0.0f * y.f),
        (x.c * y.a) + (x.d * y.c) + (0.0f * y.e),
        (x.c * y.b) + (x.d * y.d) + (0.0f * y.f),
        (x.e * y.a) + (x.f * y.c) + (1.0f * y.e),
        (x.e * y.b) + (x.f * y.d) + (1.0f * y.f),
    };
}


PgTM
pg_ident_tm()
{
    return (PgTM) {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
}


PgTM
pg_translate_tm(PgTM m, float x, float y)
{
    return pg_mul_tm(m, (PgTM) {1.0f, 0.0f, 0.0f, 1.0f, x, y});
}


PgTM
pg_scale_tm(PgTM m, float x, float y)
{
    return pg_mul_tm(m, (PgTM) {x, 0.0f, 0.0f, y, 0.0f, 0.0f});
}


PgTM
pg_rotate_tm(PgTM m, float rad)
{
    float   sinx = sinf(rad);
    float   cosx = cosf(rad);
    return pg_mul_tm(m, (PgTM) { cosx, sinx, -sinx, cosx, 0, 0 });
}

void
pg_identity(Pg *g)
{
    if (!g)
        return;

    g->s.ctm = pg_ident_tm();
}

void
pg_translate(Pg *g, float x, float y)
{
    if (!g)
        return;

    g->s.ctm = pg_translate_tm(g->s.ctm, x, y);
}

void
pg_scale(Pg *g, float x, float y)
{
    if (!g)
        return;

    g->s.ctm = pg_scale_tm(g->s.ctm, x, y);
}

void
pg_rotate(Pg *g, float rads)
{
    if (!g)
        return;

    g->s.ctm = pg_rotate_tm(g->s.ctm, rads);
}

void
pg_free(Pg *g)
{
    if (!g)
        return;

    if (g) {
        if (g->v && g->v->free)
            g->v->free(g);

        free(g->path);
        free(g);
    }
}


void
pg_resize(Pg *g, float width, float height)
{
    if (!g)
        return;

    if (g->v && g->v->resize)
        g->v->resize(g, width, height);

    g->sx = width;
    g->sy = height;
    g->s.clip_x = 0.0f;
    g->s.clip_y = 0.0f;
    g->s.clip_sx = width;
    g->s.clip_sy = height;
}


void
pg_clear(Pg *g)
{
    if (!g)
        return;

    if (!g->v || !g->v->clear)
        return;

    if (!g->s.clear)
        return;

    g->v->clear(g);
}


void
pg_fill(Pg *g)
{
    if (!g)
        return;

    if (!g->v || !g->v->fill)
        return;

    if (!g->s.fill)
        return;

    g->v->fill(g);
    pg_reset_path(g);
}


void
pg_stroke(Pg *g)
{
    if (!g)
        return;

    if (!g->v || !g->v->stroke)
        return;

    if (!g->s.stroke)
        return;

    g->v->stroke(g);
    pg_reset_path(g);
}


void
pg_fill_stroke(Pg *g)
{
    if (!g)
        return;

    if (!g->v || !g->v->fill_stroke)
        return;

    if (!g->s.fill)
        return;

    if (!g->s.stroke)
        return;

    g->v->fill_stroke(g);
    pg_reset_path(g);
}


void
pg_close(Pg *g)
{
    if (!g)
        return;

    pg_path_close(g->path);
}


void
pg_move(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_move(g->path, x, y);
}


void
pg_line(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_line(g->path, x, y);
}


void
pg_curve3(Pg *g, float bx, float by, float cx, float cy)
{
    if (!g)
        return;

    pg_path_curve3(g->path, bx, by, cx, cy);
}


void
pg_curve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy)
{
    if (!g)
        return;

    pg_path_curve4(g->path, bx, by, cx, cy, dx, dy);
}

void
pg_rmove(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_rmove(g->path, x, y);
}


void
pg_rline(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_rline(g->path, x, y);
}


void
pg_rcurve3(Pg *g, float bx, float by, float cx, float cy)
{
    if (!g)
        return;
    pg_path_rcurve3(g->path, bx, by, cx, cy);
}


void
pg_rcurve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy)
{
    if (!g)
        return;
    pg_path_rcurve4(g->path, bx, by, cx, cy, dx, dy);
}


void
pg_rectangle(Pg *g, float x, float y, float sx, float sy)
{
    if (!g)
        return;
    pg_path_rectangle(g->path, x, y, sx, sy);
}


void
pg_rounded(Pg *g, float x, float y, float sx, float sy, float rx, float ry)
{
    if (!g)
        return;
    pg_path_rounded(g->path, x, y, sx, sy, rx, ry);
}


void
pg_reset_path(Pg *g)
{
    if (!g)
        return;
    pg_path_reset(g->path);

}

void
pg_append(Pg *g, const PgPath *src)
{
    if (!g)
        return;
    if (!src)
        return;
    pg_path_append(g->path, src);
}

void
pg_set_tm(Pg *g, PgTM tm) {
    if (!g)
        return;
    g->s.ctm = tm;
}


void
pg_set_fill(Pg *g, const PgPaint *paint)
{
    if (!g)
        return;

    if (!paint)
        return;

    g->s.fill = paint;
}


void
pg_set_stroke(Pg *g, const PgPaint *paint)
{
    if (!g)
        return;

    if (!paint)
        return;

    g->s.stroke = paint;
}


void
pg_set_clear(Pg *g, const PgPaint *paint)
{
    if (!g)
        return;

    if (!paint)
        return;

    g->s.clear = paint;
}

void
pg_set_line_width(Pg *g, float line_width)
{
    if (!g)
        return;

    if (line_width == 0.0f)
        return;

    g->s.line_width = line_width;

}


void
pg_set_line_cap(Pg *g, PgLineCap line_cap)
{
    if (!g)
        return;

    switch (line_cap) {
    case PG_BUTT_CAP:
    case PG_SQUARE_CAP:
        g->s.line_cap = line_cap;
    }
}


void
pg_set_flatness(Pg *g, float flatness)
{
    if (!g)
        return;

    if (flatness < 0.0f)
        return;

    g->s.flatness = flatness;
}


void
pg_set_gamma(Pg *g, float gamma)
{
    if (!g)
        return;

    g->s.gamma = gamma;
}


void
pg_set_fill_rule(Pg *g, PgFillRule fill_rule)
{
    if (!g)
        return;

    switch (fill_rule) {
    case PG_NONZERO_RULE:
    case PG_EVEN_ODD_RULE:
        g->s.fill_rule = fill_rule;
    }

}


void
pg_set_clip(Pg *g, float x, float y, float sx, float sy)
{
    if (!g)
        return;

    g->s.clip_x = x;
    g->s.clip_y = y;
    g->s.clip_sx = sx;
    g->s.clip_sy = sy;
}


void
pg_set_text_pos(Pg *g, PgTextPos text_pos)
{
    if (!g)
        return;

    switch (text_pos) {
    case PG_TEXT_POS_TOP:
    case PG_TEXT_POS_BOTTOM:
    case PG_TEXT_POS_BASELINE:
    case PG_TEXT_POS_CENTER:
        g->s.text_pos = text_pos;
    }
}


void
pg_set_underline(Pg *g, bool underline)
{
    if (!g)
        return;

    g->s.underline = underline;
}


void
pg_reset_clip(Pg *g)
{
    if (!g)
        return;
    g->s.clip_x = 0.0f;
    g->s.clip_y = 0.0f;
    g->s.clip_sx = g->sx;
    g->s.clip_sy = g->sy;
}


PgTM
pg_get_tm(Pg *g)
{
    if (!g)
        return pg_ident_tm();
    return g->s.ctm;
}

const PgPaint *
pg_get_fill(Pg *g)
{
    if (!g)
        return 0;
    return g->s.fill;
}


const PgPaint *
pg_get_stroke(Pg *g)
{
    if (!g)
        return 0;
    return g->s.stroke;
}


const PgPaint *
pg_get_clear(Pg *g)
{
    if (!g)
        return 0;
    return g->s.clear;
}


float
pg_get_line_width(Pg *g)
{
    if (!g)
        return 0;
    return g->s.line_width;
}


PgLineCap
pg_get_line_cap(Pg *g)
{
    if (!g)
        return 0;
    return g->s.line_cap;
}


float
pg_get_flatness(Pg *g)
{
    if (!g)
        return 0;
    return g->s.flatness;
}


float
pg_get_gamma(Pg *g)
{
    if (!g)
        return 0;
    return g->s.gamma;
}


PgFillRule
pg_get_fill_rule(Pg *g)
{
    if (!g)
        return 0;
    return g->s.fill_rule;
}


PgPt
pg_get_clip_start(Pg *g)
{
    if (!g)
        return PgPt(0.0f, 0.0f);
    return PgPt(g->s.clip_x, g->s.clip_y);
}


PgPt
pg_get_clip_size(Pg *g)
{
    if (!g)
        return PgPt(0.0f, 0.0f);
    return PgPt(g->s.clip_sx, g->s.clip_sy);
}


PgTextPos
pg_get_text_pos(Pg *g)
{
    if (!g)
        return 0;
    return g->s.text_pos;
}


bool
pg_get_underline(Pg *g)
{
    if (!g)
        return 0;
    return g->s.text_pos;
}

