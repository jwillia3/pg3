#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-canvas.h>

typedef struct PgSubcanvas PgSubcanvas;
struct PgSubcanvas {
    Pg      _;
    Pg      *parent;
    float   x;
    float   y;
    float   sx;
    float   sy;
};


static
void
call(Pg *g, void subroutine(Pg *g))
{
    if (!g || !subroutine)
        return;

    PgSubcanvas     *sub = (PgSubcanvas*) g;
    Pg              *parent = sub->parent;
    PgPath          *old_path = parent->path;
    PgState         old_state = parent->s;

    parent->path = sub->_.path;
    parent->s = sub->_.s;
    pg_canvas_translate(parent, sub->x, sub->y);
    pg_canvas_set_scissor(parent,
        fmaxf(sub->x, sub->x + g->s.clip_x),
        fmaxf(sub->y, sub->y + g->s.clip_y),
        fminf(g->s.clip_sx, sub->sx),
        fminf(g->s.clip_sy, sub->sy));

    subroutine(parent);

    parent->path = old_path;
    parent->s = old_state;
}


static
void
commit(Pg *g) {
    PgSubcanvas *sub = (void*) g;
    if (sub && sub->parent && sub->parent->v->commit)
        sub->parent->v->commit(sub->parent);
}


static
void
clear(Pg *g)
{
    call(g, pg_canvas_clear);
}


static
void
fill(Pg *g)
{
    call(g, pg_canvas_fill);
}


static
void
stroke(Pg *g)
{
    call(g, pg_canvas_stroke);
}


static
void
fill_stroke(Pg *g)
{
    call(g, pg_canvas_fill_stroke);
}


static
PgPt
set_size(Pg *g, float sx, float sy)
{
    return pgpt(fminf(g->sx, sx), fminf(g->sy, sy));
}


static
void
_free(Pg *g)
{
    (void) g;
}


static const PgCanvasFunc methods = {
    .commit = commit,
    .clear = clear,
    .fill = fill,
    .stroke = stroke,
    .fill_stroke = fill_stroke,
    .set_size = set_size,
    .free = _free,
};


Pg*
pg_canvas_new_subcanvas(Pg *parent, float x, float y, float sx, float sy)
{
    if (!parent)
        return 0;

    if (sx < 0) sx = 0.0f;
    if (sy < 0) sy = 0.0f;
    sx = fmaxf(0.0f, fminf(parent->sx - x, sx));
    sy = fmaxf(0.0f, fminf(parent->sy - y, sy));

    return pgnew(PgSubcanvas,
                 _pg_canvas_init(&methods, sx, sy),
                 .parent = parent,
                 .x = truncf(x),
                 .y = truncf(y),
                 .sx = ceilf(sx),
                 .sy = ceilf(sy));
}
