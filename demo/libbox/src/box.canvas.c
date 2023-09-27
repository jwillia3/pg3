#include <pg.h>
#include <pg-box.h>
#include <pg-internal-box.h>

static void
_paint(PgBox *box, Pg *g)
{
    void (*paint)(PgBox *box, Pg *g) = pg_box_get_sys(box);
    paint(box, g);
}


PgBox*
pg_new_canvas_box(void (*paint)(PgBox *box, Pg *g), PgBoxFlags flags)
{
    if (!paint)
        return NULL;

    static const PgBoxType type = {
        .type = "CANVAS",
        .paint = _paint,
    };
    return pg_box_set_sys(pg_box_new(&type, flags), paint);
}
