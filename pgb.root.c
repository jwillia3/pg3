#include <pg3.h>
#include <pgbox.h>


static
void
dirty(Pgb *box)
{
    if (!box) return;
    pg_enqueue_redraw(pgb_get_sys(box));
}


static
void
draw(Pgb *box, Pg *g)
{
    if (!box || !g) return;

    pg_reset_state(g);
    pg_set_clear(g, pgb_color(PGB_LIGHT_COLOR));
    pg_clear(g);
    pgb_default_draw(box, g);
    pg_update(g);
}


static
PgbFn*
methods(void)
{
    static bool done;
    static PgbFn methods = {
        .draw = draw,
        .dirty = dirty,
    };
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}



Pgb*
pgb_root(Pg *g)
{
    if (!g)
        return 0;

    Pgb *box = pgb_box(0.0f, 0.0f, PGB_FILL, methods());
    pg_set_box(g, box);
    pgb_set_sys(box, g);
    return box;
}
