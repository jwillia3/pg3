#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <pg3.h>
#include <internal.h>

static const PgPaint black_paint = {
    .type = PG_SOLID_PAINT,
    .cspace = PG_SRGB,
    .colors[0] = { 0.0f, 0.0f, 0.0f, 1.0f },
    .nstops = 1,
};

static const PgPaint white_paint = {
    .type = PG_SOLID_PAINT,
    .cspace = PG_SRGB,
    .colors[0] = { 1.0f, 1.0f, 1.0f, 1.0f },
    .nstops = 1,
};

Pg
pg_init_canvas(const PgCanvasImpl *v, float width, float height)
{
    Pg canvas = {
                    .v = v,
                    .size = PgPt(width, height),
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
        .clip_x = 0.0f,
        .clip_y = 0.0f,
        .clip_sx = g->size.x,
        .clip_sy = g->size.y,
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

    g->size = PgPt(width, height);
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
    pg_path_reset(g->path);
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
    pg_path_reset(g->path);
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
    pg_path_reset(g->path);
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
pg_reset(Pg *g)
{
    if (!g)
        return;
    pg_path_reset(g->path);

}


