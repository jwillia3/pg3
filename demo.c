#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pg3.h>

static const float font_size = 100.0f;

int
main()
{
    setvbuf(stdout, 0, _IONBF, 0);

    Pg *g = pg_window(1024, 768, "Demo");
    PgFont *font = pg_find_font("FreeSans", 600, 0);
    pg_scale_font(font,
        font_size * 1.0f / 72.0f * pg_dpi().x,
        font_size * 1.0f / 72.0f * pg_dpi().y);

    while (pg_wait()) {
        pg_clear(g);
        pg_string_path(g,
                       font,
                       pg_font_height(font),
                       pg_font_height(font),
                       "PLAIN GRAPHICS");
        pg_fill(g);
        pg_update();
    }

    puts("done.");
}
