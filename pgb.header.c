#include <pg3.h>
#include <pgbox.h>

Pgb *pgb_header(const char *text, PgbFlags flags)
{
    return pgb_set_label_font(pgb_label(text, flags),
                              PGB_HEADER_FONT);
}
