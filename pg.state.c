#include <pg3.h>


bool
pg_save(Pg *g)
{
    if (!g)
        return false;

    if (g->nsaved + 1 > sizeof g->saved / sizeof *g->saved)
        return false;

    g->saved[g->nsaved++] = g->s;
    return true;
}


bool
pg_restore(Pg *g)
{
    if (!g)
        return false;

    if (!g->nsaved)
        return false;

    g->s = g->saved[--g->nsaved];
    return true;
}
