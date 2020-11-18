#include <stdlib.h>
#include <pg3.h>
#include "internal.h"

void
pg_shutdown(void)
{
    _pg_free_font_list();
}
