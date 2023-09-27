#include <pg.h>
#include <pg-box.h>
#include <pg-internal-box.h>


PgBox*
pg_box_pad(void)
{
    const static PgBoxType type = {
        .type = "PAD",
    };
    return pg_box_new(&type, 0);
}


PgBox*
pg_box_fill(void)
{
    const static PgBoxType type = {
        .type = "FILL",
    };
    return pg_box_new(&type, PG_BOX_FLAG_FILL);
}
