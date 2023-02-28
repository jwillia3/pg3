#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

static
PgbFn*
methods(void)
{
    static bool done;
    static PgbFn methods = {};
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_space(float cx, float cy, PgbFlags flags)
{
    return pgb_box(cx, cy, flags, methods());
}


Pgb*
pgb_pad(void)
{
    return pgb_space(PAD, PAD, 0);
}


Pgb*
pgb_fill(void)
{
    return pgb_space(0, 0, PGB_FILL);
}
