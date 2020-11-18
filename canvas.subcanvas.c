#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pg3.h>
#include "internal.h"

typedef struct Sub Sub;

struct Sub {
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
    Sub     *sub = (Sub*) g;
    Pg      *parent = sub->parent;
    PgPath  *old_path = parent->path;

    parent->path = sub->_.path;
    parent->s = sub->_.s;
    pg_translate(parent, sub->x, sub->y);
    pg_set_clip(parent,
        sub->x + g->s.clip_x,
        sub->y + g->s.clip_y,
        fminf(g->s.clip_sx, sub->sx),
        fminf(g->s.clip_sy, sub->sy));

    if (subroutine)
        subroutine(parent);

    parent->path = old_path;
}


static
void
clear(Pg *g)
{
    call(g, pg_clear);
}


static
void
fill(Pg *g)
{
    call(g, pg_fill);
}


static
void
stroke(Pg *g)
{
    call(g, pg_stroke);
}


static
void
fill_stroke(Pg *g)
{
    call(g, pg_fill_stroke);
}


static
void
resize(Pg *g, float sx, float sy)
{
    (void) g;
    (void) sx;
    (void) sy;
}


static
void
_free(Pg *g)
{
    (void) g;
}


static const PgCanvasImpl methods = {
    .clear = clear,
    .fill = fill,
    .stroke = stroke,
    .fill_stroke = fill_stroke,
    .resize = resize,
    .free = _free,
};


Pg*
pg_subcanvas(Pg *parent, float x, float y, float sx, float sy)
{
    if (!parent)
        return 0;

    return new(Sub,
        pg_init_canvas(&methods, sx, sy),
        .parent = parent,
        .x = x,
        .y = y,
        .sx = sx,
        .sy = sy);
}
