#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pg3.h>

#define BIG     (24.0f * 1/72.0f * pg_dpi().y)
#define SMALL   (12.0f * 1/72.0f * pg_dpi().y)

static
bool
has_fixed(PgFamily *fam)
{
    for (PgFace *face = fam->faces; face->family; face++)
        if (face->is_fixed)
            return true;
    return false;
}


static
void
big(PgFont *font, const char *text)
{
    pg_scale_font(font, BIG, BIG);
    Pg *g = pg_ctrl(pg_measure_string(font, text), pg_font_height(font));

    pg_string_path(g, font, 0.0f, 0.0f, text);
    pg_fill(g);
}


static
void
small(PgFont *font, const char *text)
{
    pg_scale_font(font, SMALL, SMALL);
    Pg *g = pg_ctrl(pg_measure_string(font, text), pg_font_height(font));

    pg_string_path(g, font, 0.0f, 0.0f, text);
    pg_fill(g);
}


static
void
specimen(PgFont *font)
{
    PgFamily *fam = pg_list_fonts();
    while (fam->name && strcmp(fam->name, pg_font_string(font, PG_FONT_FAMILY)))
        fam++;

    pg_group(false);

    big(font, pg_font_string(font, PG_FONT_FAMILY));

    const char *type = pg_font_int(font, PG_FONT_IS_SANS_SERIF)? "Sans Serif":
                       pg_font_int(font, PG_FONT_IS_SERIF)? "Serif":
                       "Other";
    small(font, type);

    if (fam)
        for (PgFace *face = fam->faces; face->family; face++)
            small(font, face->style);



    pg_end_group();
}


int
main()
{
    setvbuf(stdout, 0, _IONBF, 0);


    Pg      *g = pg_window(1024, 768, "Demo");
    char    selected[256] = "Family";
    bool    fixed_only = true;
    PgFont  *ui_font = pg_font();
    PgFont  *target_font = 0;
    float   vscroll = .0f;


    if (pg_list_fonts()->name)
        strcpy(selected, pg_list_fonts()->name);

    while (pg_event()) {
        pg_clear(g);

        pg_checkbox("Fixed Only", &fixed_only);

        pg_group(true);
        {

            // Font Selection List Box.
            float   nfonts = 0;
            float   max_width = .0f;
            float   total_height = .0f;

            for (PgFamily *fam = pg_list_fonts(); fam->name; fam++) {
                if (fixed_only && !has_fixed(fam))
                    continue;

                nfonts++;
                max_width = fmaxf(max_width, pg_measure_string(ui_font, fam->name));
            }

            total_height = pg_font_height(ui_font) * nfonts;

            pg_vscroll(max_width, 600.0f, total_height, &vscroll);
            {
                pg_set_group_pad(.0f, .0f);

                for (PgFamily *fam = pg_list_fonts(); fam->name; fam++) {
                    if (fixed_only && !has_fixed(fam))
                        continue;

                    if (pg_item(max_width, fam->name, !strcmp(selected, fam->name))) {
                        strcpy(selected, fam->name);
                        pg_redraw();
                    }
                }
            }
            pg_end_group();


            // Font Display.

            const char *current = pg_font_string(target_font, PG_FONT_FAMILY);

            if (!target_font || strcmp(selected, current)) {
                pg_free_font(target_font);
                target_font = pg_find_font(selected, 400, false);
            }

            specimen(target_font);
        }

        pg_end_group();


        pg_update();
    }
}
