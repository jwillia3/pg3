#include <pg3.h>
#include <pgbox.h>

Pgb*
pgb_group(Pgb *content, PgbFlags flags)
{
    return pgb_add(pgb_space(0, 0, flags), content);
}

Pgb*
pgb_grouped(PgbFlags flags, Pgb **content)
{
    return pgb_add_list(pgb_space(0, 0, flags), content);
}


Pgb*
pgb_hgroup(Pgb *content, PgbFlags flags)
{
    return pgb_add(pgb_space(0, 0, flags | PGB_PACK_HORIZ), content);
}

Pgb*
pgb_hgrouped(PgbFlags flags, Pgb **content)
{
    return pgb_add_list(pgb_space(0, 0, flags | PGB_PACK_HORIZ), content);
}
