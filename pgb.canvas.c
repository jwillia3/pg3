#include <pg3.h>
#include <pgbox.h>


static
void
draw(Pgb *box, Pg *g)
{
    void (*draw)(Pgb *box, Pg *g) = (void (*) (Pgb*, Pg*)) pgb_get_sys(box);
    draw(box, g);
    pgb_default_draw(box, g);
}


static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = {
        .draw = draw,
    };
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_canvas(float x, float y, void (*draw)(Pgb *box, Pg *g), PgbFlags flags)
{
    if (!draw)
        return 0;
    return pgb_set_sys(pgb_box(x, y, flags, methods()), draw);
}
