#include <math.h>
#include <pg3.h>


PgPaint
pg_solid(PgColorSpace cspace, float u, float v, float w, float a)
{
    return (PgPaint) {
        .type = PG_SOLID_PAINT,
        .cspace = cspace,
        .colors[0] = { u, v, w, a },
        .nstops = 1,
    };
}


PgPaint
pg_linear(PgColorSpace cspace, float ax, float ay, float bx, float by)
{
    return (PgPaint) {
        .type = PG_LINEAR_PAINT,
        .cspace = cspace,
        .colors[0] = (PgColor) {0.0f, 0.0f, 0.0f, 1.0f},
        .stops[0] = 0.0f,
        .nstops = 0,
        .a = PgPt(ax, ay),
        .b = PgPt(bx, by),
        .ra = .0f,
    };
}


void
pg_add_stop(PgPaint *paint, float t, float u, float v, float w, float a)
{
    if (!paint)
        return;

    if (paint->nstops + 1 >= 8)
        return;


    // Do not allow inserting out of order.
    float last = paint->nstops
                    ? paint->stops[paint->nstops - 1]
                    : 0.0f;

    if (last > t || t > 1.0f)
        return;

    // Add small distance if t is same as last.
    if (paint->nstops && last == t)
        t = nextafterf(last, last + 1.0f);

    paint->stops[paint->nstops] = t;
    paint->colors[paint->nstops] = (PgColor) { u, v, w, a };
    paint->nstops++;
}
