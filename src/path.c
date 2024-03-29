#include <math.h>
#include <float.h>
#include <stdlib.h>
#include <string.h>
#include <pg3/pg.h>
#include <pg3/pg-internal-canvas.h>



static size_t
capacity(size_t n)
{
    return (n + 31UL) & ~31UL;
}


static PgPartType
previous(PgPath *path)
{
    if (!path->nparts)
        return PG_PART_CLOSE;

    return path->parts[path->nparts - 1].type;
}


static
unsigned
partcount(PgPartType type)
{
    switch (type) {
    case PG_PART_MOVE:      return 1;
    case PG_PART_LINE:      return 1;
    case PG_PART_CURVE3:    return 2;
    case PG_PART_CURVE4:    return 3;
    case PG_PART_CLOSE:     return 0;
    }
    return 0;
}



static void
addpart(PgPath *path,
        PgPartType type,
        float ax,
        float ay,
        float bx,
        float by,
        float cx,
        float cy)
{
    if (!path)
        return;

    if (path->nparts + 1 >= capacity(path->nparts)) {
        size_t new = capacity(path->nparts + 1);
        path->parts = realloc(path->parts, new * sizeof *path->parts);
    }

    PgPart part = { type, {{ax, ay}, {bx, by}, {cx, cy} } };
    path->parts[path->nparts++] = part;

    if (partcount(part.type))
        path->cur = part.pt[partcount(part.type) - 1];
}


PgPath *
pg_path_new(void)
{
    return pgnew(PgPath,
                .nparts = 0,
                .parts = 0,
                .cur = pgpt(0, 0));
}


void
pg_path_free(PgPath *path)
{
    if (!path)
        return;

    free(path->parts);
    free(path);
}

void
pg_path_reset(PgPath *path)
{
    if (!path)
        return;

    path->nparts = 0;
}

void
pg_path_append(PgPath *path, const PgPath *src)
{
    if (!path)
        return;
    if (!src)
        return;

    for (unsigned i = 0; i < src->nparts; i++) {
        PgPt *p = src->parts[i].pt;
        switch (src->parts[i].type) {
        case PG_PART_MOVE:
            pg_path_move(path, p[0].x, p[0].y);
            break;
        case PG_PART_LINE:
            pg_path_line(path, p[0].x, p[0].y);
            break;
        case PG_PART_CURVE3:
            pg_path_curve3(path, p[0].x, p[0].y, p[1].x, p[1].y);
            break;
        case PG_PART_CURVE4:
            pg_path_curve4(path, p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
            break;
        case PG_PART_CLOSE:
            pg_path_close(path);
            break;
        }
    }
}


void
pg_path_move(PgPath *path, float x, float y)
{
    addpart(path, PG_PART_MOVE, x, y, 0.0f, 0.0f, 0.0f, 0.0f);
}

void
pg_path_line(PgPath *path, float x, float y)
{
    addpart(path, PG_PART_LINE, x, y, 0.0f, 0.0f, 0.0f, 0.0f);
}


void
pg_path_curve3(PgPath *path, float bx, float by, float cx, float cy)
{
    addpart(path, PG_PART_CURVE3, bx, by, cx, cy, 0.0f, 0.0f);
}


void
pg_path_curve4(PgPath *path, float bx, float by, float cx, float cy, float dx, float dy)
{
    addpart(path, PG_PART_CURVE4, bx, by, cx, cy, dx, dy);
}

void
pg_path_rmove(PgPath *path, float x, float y)
{
    if (!path)
        return;

    x += path->cur.x;
    y += path->cur.y;
    addpart(path, PG_PART_MOVE, x, y, 0.0f, 0.0f, 0.0f, 0.0f);
}


void
pg_path_rline(PgPath *path, float x, float y)
{
    if (!path)
        return;

    x += path->cur.x;
    y += path->cur.y;
    addpart(path, PG_PART_LINE, x, y, 0.0f, 0.0f, 0.0f, 0.0f);
}

void
pg_path_rcurve3(PgPath *path, float bx, float by, float cx, float cy)
{
    if (!path)
        return;

    bx += path->cur.x;
    by += path->cur.y;
    cx += bx;
    cy += by;
    addpart(path, PG_PART_CURVE3, bx, by, cx, cy, 0.0f, 0.0f);
}


void
pg_path_rcurve4(PgPath *path, float bx, float by, float cx, float cy, float dx, float dy)
{
    if (!path)
        return;

    bx += path->cur.x;
    by += path->cur.y;
    cx += bx;
    cy += by;
    dx += cx;
    dy += cy;
    addpart(path, PG_PART_CURVE4, bx, by, cx, cy, dx, dy);
}


void
pg_path_rectangle(PgPath *path, float x, float y, float sx, float sy)
{
    pg_path_move(path, x, y);
    pg_path_rline(path, sx, 0.0f);
    pg_path_rline(path, 0.0f, sy);
    pg_path_rline(path, -sx, 0.0f);
    pg_path_close(path);
}


void
pg_path_rounded_rectangle(PgPath *path, float x, float y, float sx, float sy, float r)
{
    if (r == 0.0f && r == 0.0f) {
        pg_path_rectangle(path, x, y, sx, sy);
        return;
    }

    if (r >= fminf(sx, sy) * .5f)
        // - .25f is a hack to prevent overlapping lines.
        r = fminf(sx, sy) * .5f - .25f;

    sx -= r * 2.0f;
    sy -= r * 2.0f;

    pg_path_move(path, x + sx + r, y);
    pg_path_rcurve3(path, r, .0f, .0f, r);
    pg_path_rline(path, .0f, sy);
    pg_path_rcurve3(path, .0f, r, -r, .0f);
    pg_path_rline(path, -sx, .0f);
    pg_path_rcurve3(path, -r, .0f, .0f, -r);
    pg_path_rline(path, .0f, -sy);
    pg_path_rcurve3(path, .0f, -r, r, .0f);
    pg_path_close(path);
}


void
pg_path_close(PgPath *path)
{
    if (previous(path) != PG_PART_CLOSE)
        addpart(path, PG_PART_CLOSE, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
}
