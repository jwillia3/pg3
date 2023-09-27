#include <pg.h>
#include <pg-box.h>
#include <pg-internal-box.h>

PgBox*
pg_box_new_group(PgBoxFlags flags)
{
    const static PgBoxType type = {
        .type = "GROUP",
    };
    return pg_box_new(&type, flags);
}


PgBox*
pg_box_new_vgroup(PgBoxFlags flags)
{
    return pg_box_new_group(flags | PG_BOX_FLAG_VERT);
}


PgBox*
pg_box_grouped(PgBoxFlags flags, PgBox **children)
{
    return pg_box_added(pg_box_new_group(flags), children);
}


PgBox*
pg_box_vgrouped(PgBoxFlags flags, PgBox **children)
{
    return pg_box_added(pg_box_new_vgroup(flags), children);
}
