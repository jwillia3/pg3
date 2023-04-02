#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-box.h>

typedef struct {
    char        *text;
    PgFont      *font;
    PgPaint     *fg;
} Info;


static Info*
get_info(const PgBox *box)
{
    return pg_box_get_sys(box);
}


static void
paint(PgBox *box, Pg *g)
{
    Info        *info = get_info(box);
    pg_canvas_set_fill(g, info->fg);
    pg_canvas_show_string(g, info->font, 0, 0, info->text);
}


static PgPt
measure(const PgBox *box)
{
    Info        *info = get_info(box);
    PgFont      *font = info->font;
    const char  *text = info->text;
    float       width = pg_font_measure_string(font, text);
    float       height = pg_font_get_height(font);
    return pgpt(ceilf(width), ceilf(height));
}


static void
_free(PgBox *box)
{
    Info *info = get_info(box);
    pg_font_free(info->font);
    free(info->text);
    free(info);
}


static PgBox*
setup(PgBoxFlags flags, const char *text, PgFont *font)
{
    static const PgBoxType type = {
        .type = "TEXT",
        .paint = paint,
        .measure = measure,
        .free = _free,
    };

    if (!text)
        text = "";

    return pg_box_set_sys(pg_box_new(&type, flags),
        pgnew(Info,
            .font = pg_font_clone(font),
            .fg = pg_paint_clone(pg_paint_from_name("ui-fg")),
            .text = strdup(text)));
}


PgBox*
pg_box_text(PgBoxFlags flags, const char *text)
{
    return setup(flags, text, pg_box_font_text());
}


PgBox*
pg_box_title(PgBoxFlags flags, const char *text)
{
    return setup(flags, text, pg_box_font_title());
}


PgBox*
pg_box_subtitle(PgBoxFlags flags, const char *text)
{
    return setup(flags, text, pg_box_font_subtitle());
}


PgBox*
pg_box_heading(PgBoxFlags flags, const char *text)
{
    return setup(flags, text, pg_box_font_heading());
}


const char*
pg_text_box_get_text(PgBox *box)
{
    Info *info = get_info(box);
    return info? info->text: NULL;
}


PgFont*
pg_text_box_get_font(PgBox *box)
{
    Info *info = get_info(box);
    return info? info->font: NULL;
}



PgPaint*
pg_text_box_get_fg(PgBox *box)
{
    Info *info = get_info(box);
    return info? info->fg: NULL;
}


PgBox*
pg_box_text_set_text(PgBox *box, const char *text)
{
    Info *info = get_info(box);
    if (info) {
        free(info->text);
        info->text = strdup(text? text: "");
    }
    return box;
}


PgBox*
pg_box_text_set_font(PgBox *box, PgFont *font)
{
    Info *info = get_info(box);
    if (info && font) {
        pg_font_free(info->font);
        info->font = pg_font_clone(font);
    }
    return box;
}


PgBox*
pg_box_text_set_fg(PgBox *box, PgPaint *paint)
{
    Info *info = get_info(box);
    if (info) {
        pg_paint_free(paint);
        info->fg = pg_paint_clone(paint);
    }
    return box;
}