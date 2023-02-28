#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

typedef struct Label Label;
struct Label {
    char        *text;
    PgbFont     font;
    PgbColor    color;
};


Pgb*
pgb_set_label_text(Pgb *box, const char *text)
{
    Label *lab = pgb_get_sys(box);
    if (!lab || !text) return box;

    free(lab->text);
    lab->text = strdup(text);
    pgb_dirty(box);
    return box;
}


const char*
pgb_get_label_text(Pgb *box)
{
    Label *lab = pgb_get_sys(box);
    return lab? lab->text: 0;
}


Pgb*
pgb_set_label_font(Pgb *box, PgbFont font)
{
    Label *lab = pgb_get_sys(box);
    if (lab) lab->font = font;
    return box;
}


PgbFont
pgb_get_label_size(Pgb *box)
{
    Label *lab = pgb_get_sys(box);
    return lab? lab->font: 0.0f;
}


Pgb*
pgb_set_label_color(Pgb *box, PgbColor color)
{
    Label *lab = pgb_get_sys(box);
    if (lab) lab->color = color;
    return box;
}


PgbColor
pgb_get_label_color(Pgb *box)
{
    Label *lab = pgb_get_sys(box);
    return lab? lab->color: 0.0f;
}


static
void
draw(Pgb *box, Pg *g)
{
    Label       *lab = pgb_get_sys(box);
    const char  *text = lab->text;
    PgFont      *font = pgb_font(lab->font);
    float       cx = ceilf(pg_measure_string(font, text));
    float       cy = ceilf(pg_font_height(font));
    pg_string_path(g,
                   font,
                   (box->sx - cx) / 2.0f,
                   (box->sy - cy) / 2.0f,
                   text);
    pg_set_fill(g, pgb_color(lab->color));
    pg_fill(g);
}


static
void
autosize(Pgb *box)
{
    Label       *lab = pgb_get_sys(box);
    const char  *text = lab->text;
    PgFont      *font = pgb_font(lab->font);

    box->cx = ceilf(pg_measure_string(font, text));
    box->cy = ceilf(pg_font_height(font));
}


static
void
_free(Pgb *box)
{
    Label *lab = pgb_get_sys(box);
    free(lab->text);
    free(lab);
}


static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = {
       .draw = draw,
       .free = _free,
       .autosize = autosize,
    };
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_label(const char *text, PgbFlags flags)
{
    if (!text) return 0;
    Label *lab = new(Label,
                     .text = strdup(text),
                     .font = PGB_DEFAULT_FONT,
                     .color = PGB_DARK_COLOR);
    return pgb_set_sys(pgb_box(0.0f, 0.0f, flags, methods()), lab);
}
