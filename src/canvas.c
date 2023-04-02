#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <pg.h>
#include <pg-internal-canvas.h>
#include <pg-internal-platform.h>


PgPt
pg_canvas_get_size(Pg *g)
{
    if (!g)
        return pgpt(0, 0);
    return pgpt(g->sx, g->sy);
}



void
pg_canvas_set_size(Pg *g, float width, float height)
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



Pg
_pg_canvas_init(const PgCanvasFunc *v, float width, float height)
{
    Pg canvas = {
                    .v = v,
                    .sx = width,
                    .sy = height,
                    .path = pg_path_new(),
                };

    pg_canvas_state_reset(&canvas);

    return canvas;
}


void
pg_canvas_state_reset(Pg *g)
{
    if (!g)
        return;

    const PgPaint   *black = pg_paint_from_name("black");
    const PgPaint   *white = pg_paint_from_name("white");

    g->s = (PgState) {
        .ctm = pg_mat_identity(),
        .clear = white,
        .fill = black,
        .stroke = black,
        .line_width = 1.0f,
        .line_cap = PG_BUTT_CAP,
        .flatness = 1.0f,
        .fill_rule = PG_NONZERO_RULE,
        .gamma = 1.8f,
        .clip_x = 0.0f,
        .clip_y = 0.0f,
        .clip_sx = g->sx,
        .clip_sy = g->sy,
        .underline = false,
    };
}


PgPt
pg_mat_apply(PgTM ctm, PgPt p)
{
    return pgpt(ctm.a * p.x + ctm.c * p.y + ctm.e,
                ctm.b * p.x + ctm.d * p.y + ctm.f);
}


PgTM
pg_mat_multiply(PgTM x, PgTM y)
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
pg_mat_identity()
{
    return (PgTM) {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
}


PgTM
pg_mat_translate(PgTM m, float x, float y)
{
    return pg_mat_multiply(m, (PgTM) {1.0f, 0.0f, 0.0f, 1.0f, x, y});
}


PgTM
pg_mat_scale(PgTM m, float x, float y)
{
    return pg_mat_multiply(m, (PgTM) {x, 0.0f, 0.0f, y, 0.0f, 0.0f});
}


PgTM
pg_mat_rotate(PgTM m, float rad)
{
    float   sinx = sinf(rad);
    float   cosx = cosf(rad);
    return pg_mat_multiply(m, (PgTM) { cosx, sinx, -sinx, cosx, 0, 0 });
}

void
pg_canvas_identity(Pg *g)
{
    if (!g)
        return;

    g->s.ctm = pg_mat_identity();
}

void
pg_canvas_translate(Pg *g, float x, float y)
{
    if (!g)
        return;

    g->s.ctm = pg_mat_translate(g->s.ctm, x, y);
}

void
pg_canvas_scale(Pg *g, float x, float y)
{
    if (!g)
        return;

    g->s.ctm = pg_mat_scale(g->s.ctm, x, y);
}

void
pg_canvas_rotate(Pg *g, float rads)
{
    if (!g)
        return;

    g->s.ctm = pg_mat_rotate(g->s.ctm, rads);
}

void
pg_canvas_free(Pg *g)
{
    if (!g)
        return;


    if (g->v && g->v->free)
        g->v->free(g);

    for (PgState *n, *c = g->saved; c; c = n)
        n = c->next,
        free(c);

    free(g->path);
    free(g);
}


bool
pg_canvas_state_save(Pg *g)
{
    if (!g)
        return false;

    PgState *save = malloc(sizeof *save);
    *save = g->s;
    save->next = g->saved;
    g->saved = save;
    return true;
}


bool
pg_canvas_state_restore(Pg *g)
{
    if (!g && !g->saved)
        return false;

    PgState *doomed = g->saved;
    g->saved = doomed->next;
    g->s = *doomed;
    free(doomed);
    return true;
}


void
pg_canvas_commit(Pg *g)
{
    if (g && g->v && g->v->commit)
        g->v->commit(g);
}


void
pg_canvas_clear(Pg *g)
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
pg_canvas_fill(Pg *g)
{
    if (!g)
        return;

    if (!g->v || !g->v->fill)
        return;

    if (!g->s.fill)
        return;

    g->v->fill(g);
    pg_canvas_path_clear(g);
}


void
pg_canvas_stroke(Pg *g)
{
    if (!g)
        return;

    if (!g->v || !g->v->stroke)
        return;

    if (!g->s.stroke)
        return;

    g->v->stroke(g);
    pg_canvas_path_clear(g);
}


void
pg_canvas_fill_stroke(Pg *g)
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
    pg_canvas_path_clear(g);
}


void
pg_canvas_close_path(Pg *g)
{
    if (!g)
        return;

    pg_path_close(g->path);
}


void
pg_canvas_move(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_move(g->path, x, y);
}


void
pg_canvas_line(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_line(g->path, x, y);
}


void
pg_canvas_curve3(Pg *g, float bx, float by, float cx, float cy)
{
    if (!g)
        return;

    pg_path_curve3(g->path, bx, by, cx, cy);
}


void
pg_canvas_curve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy)
{
    if (!g)
        return;

    pg_path_curve4(g->path, bx, by, cx, cy, dx, dy);
}

void
pg_canvas_rmove(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_rmove(g->path, x, y);
}


void
pg_canvas_rline(Pg *g, float x, float y)
{
    if (!g)
        return;

    pg_path_rline(g->path, x, y);
}


void
pg_canvas_rcurve3(Pg *g, float bx, float by, float cx, float cy)
{
    if (!g)
        return;
    pg_path_rcurve3(g->path, bx, by, cx, cy);
}


void
pg_canvas_rcurve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy)
{
    if (!g)
        return;
    pg_path_rcurve4(g->path, bx, by, cx, cy, dx, dy);
}


void
pg_canvas_rectangle(Pg *g, float x, float y, float sx, float sy)
{
    if (!g)
        return;
    pg_path_rectangle(g->path, x, y, sx, sy);
}


void
pg_canvas_rounded_rectangle(Pg *g, float x, float y, float sx, float sy, float rx, float ry)
{
    if (!g)
        return;
    pg_path_rounded(g->path, x, y, sx, sy, rx, ry);
}


void
pg_canvas_path_clear(Pg *g)
{
    if (!g)
        return;
    pg_path_reset(g->path);

}

void
pg_canvas_append_path(Pg *g, const PgPath *src)
{
    if (!g)
        return;
    if (!src)
        return;
    pg_path_append(g->path, src);
}

void
pg_canvas_set_mat(Pg *g, PgTM tm) {
    if (!g)
        return;
    g->s.ctm = tm;
}


void
pg_canvas_set_fill(Pg *g, const PgPaint *paint)
{
    if (!g)
        return;

    if (!paint)
        return;

    g->s.fill = paint;
}


void
pg_canvas_set_stroke(Pg *g, const PgPaint *paint)
{
    if (!g)
        return;

    if (!paint)
        return;

    g->s.stroke = paint;
}


void
pg_canvas_set_clear(Pg *g, const PgPaint *paint)
{
    if (!g)
        return;

    if (!paint)
        return;

    g->s.clear = paint;
}

void
pg_canvas_set_line_width(Pg *g, float line_width)
{
    if (!g)
        return;

    if (line_width == 0.0f)
        return;

    g->s.line_width = line_width;

}


void
pg_canvas_set_line_cap(Pg *g, PgLineCap line_cap)
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
pg_canvas_set_fill_flatness(Pg *g, float flatness)
{
    if (!g)
        return;

    if (flatness < 0.0f)
        return;

    g->s.flatness = flatness;
}


void
pg_canvas_set_gamma(Pg *g, float gamma)
{
    if (!g)
        return;

    g->s.gamma = gamma;
}


void
pg_canvas_set_fill_rule(Pg *g, PgFillRule fill_rule)
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
pg_canvas_set_scissors(Pg *g, float x, float y, float sx, float sy)
{
    if (!g)
        return;

    g->s.clip_x = x;
    g->s.clip_y = y;
    g->s.clip_sx = sx;
    g->s.clip_sy = sy;
}


void
pg_canvas_set_underline(Pg *g, bool underline)
{
    if (!g)
        return;

    g->s.underline = underline;
}


void
pg_canvas_scissor(Pg *g)
{
    if (!g)
        return;
    g->s.clip_x = 0.0f;
    g->s.clip_y = 0.0f;
    g->s.clip_sx = g->sx;
    g->s.clip_sy = g->sy;
}


PgTM
pg_canvas_get_mat(Pg *g)
{
    if (!g)
        return pg_mat_identity();
    return g->s.ctm;
}

const PgPaint *
pg_canvas_get_fill(Pg *g)
{
    if (!g)
        return 0;
    return g->s.fill;
}


const PgPaint *
pg_canvas_get_stroke(Pg *g)
{
    if (!g)
        return 0;
    return g->s.stroke;
}


const PgPaint *
pg_canvas_get_clear(Pg *g)
{
    if (!g)
        return 0;
    return g->s.clear;
}


float
pg_canvas_get_line_width(Pg *g)
{
    if (!g)
        return 0;
    return g->s.line_width;
}


PgLineCap
pg_canvas_get_line_cap(Pg *g)
{
    if (!g)
        return 0;
    return g->s.line_cap;
}


float
pg_canvas_get_flatness(Pg *g)
{
    if (!g)
        return 0;
    return g->s.flatness;
}


float
pg_canvas_get_gamma(Pg *g)
{
    if (!g)
        return 0;
    return g->s.gamma;
}


PgFillRule
pg_canvas_get_fill_rule(Pg *g)
{
    if (!g)
        return 0;
    return g->s.fill_rule;
}


PgPt
pg_canvas_get_scissor_start(Pg *g)
{
    if (!g)
        return pgpt(0.0f, 0.0f);
    return pgpt(g->s.clip_x, g->s.clip_y);
}


PgPt
pg_canvas_get_scissor_size(Pg *g)
{
    if (!g)
        return pgpt(0.0f, 0.0f);
    return pgpt(g->s.clip_sx, g->s.clip_sy);
}


bool
pg_canvas_get_underline(Pg *g)
{
    if (!g)
        return 0;
    return g->s.underline;
}

