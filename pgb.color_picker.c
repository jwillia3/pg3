#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

const float SWATCHX = 32.0f;
const float SWATCHY = 32.0f;

typedef struct Pick Pick;

struct Pick {
    PgColorSpace    cspace;
    PgColor         color;
    PgColor         variants[9];
    PgColor         lscale[11];
    PgColor         cscale[11];
    PgColor         hscale[11];
    Pgb             *output;
    Pgb             *rgb;
    Pgb             *ok;
    void            (*done)(Pgb *box, PgColorSpace cspace, PgColor color);
};


static
float
clamp(float x)
{
    return fminf(1.0f, fmaxf(0.0f, x));
}


static
void
pop_swatches(Pgb *box)
{
    Pick        *pick = box->sys;
    PgColor     *set = pick->variants;
    PgColor     c = pick->color;

    const float STEP = 0.05f;
    /*
        0 1 2
        3 4 5
        6 7 8
     */
    set[0] = (PgColor) { clamp(c.x - STEP), c.y, c.z, c.a };
    set[3] = (PgColor) { clamp(c.x + STEP), c.y, c.z, c.a };
    set[6] = (PgColor) { clamp(c.x + STEP * 2.0f), c.y, c.z, c.a };

    set[1] = (PgColor) { c.x, clamp(c.y - STEP), c.z, c.a };
    set[4] = (PgColor) { c.x, clamp(c.y + STEP), c.z, c.a };
    set[7] = (PgColor) { c.x, clamp(c.y + STEP * 2.0f), c.z, c.a };

    set[2] = (PgColor) { c.x, c.y, clamp(c.z - STEP), c.a };
    set[5] = (PgColor) { c.x, c.y, clamp(c.z + STEP), c.a };
    set[8] = (PgColor) { c.x, c.y, clamp(c.z + STEP * 2.0f), c.a };

    for (int i = 0; i < 11; i++) {
        pick->lscale[i] = (PgColor) { i / 10.0f, c.y, c.z, c.a };
        pick->cscale[i] = (PgColor) { c.x, i / 10.0f, c.z, c.a };
        pick->hscale[i] = (PgColor) { c.x, c.y, i / 10.0f, c.a };
    }

    char tmp[32];
    snprintf(tmp, sizeof tmp, "lch(%g, %g, %g)", c.x, c.y, c.z);
    pgb_set_textbox_text(pick->output, tmp);

    PgColor rgb = pg_convert_color_to_rgb(pick->cspace, pick->color, 2.2f);
    snprintf(tmp, sizeof tmp, "rgb(%0.1f, %0.1f, %0.1f)", rgb.x, rgb.y, rgb.z);
    pgb_set_textbox_text(pick->rgb, tmp);
}


static
void
ok(Pgb *box)
{
    Pgb *target = box->user;
    Pick *pick = target->sys;
    if (pick->done)
        pick->done(target, pick->cspace, pick->color);
}


static
void
swatch_mouse_down(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) x, (void) y, (void) mods;
    if (button == 0 && !mods) {
        Pgb *target = box->user;
        Pick *pick = target->sys;
        pick->color = *(PgColor *) box->sys;
        pop_swatches(target);
        pgb_dirty(target);
    }
}


static
void
swatch_draw(Pgb *box, Pg *g)
{
    Pgb     *target = box->user;
    Pick    *pick = target->sys;
    PgColor *c = box->sys;
    PgPaint paint = pg_solid(pick->cspace, c->x,  c->y, c->z, 1.0f);
    pg_set_clear(g, &paint);
    pg_clear(g);
}


static
PgbFn*
swatch_methods(void) {
    static bool done;
    static PgbFn methods = {
        .mouse_down = swatch_mouse_down,
        .draw = swatch_draw,
    };
    if (done) return &methods;
    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


static
Pgb*
new_swatch(float sx, float sy, Pgb *target, PgColor *color, PgbFlags flags)
{
    Pgb *box = pgb_box(sx, sy, flags, swatch_methods());
    box->sys = color;
    box->user = target;
    return box;
}


static
void
setup(Pgb *box)
{
    Pick *pick = box->sys;

    Pgb *lscale = pgb_grouped(PGB_CENTER_CROSS, 0);
    Pgb *cscale = pgb_grouped(PGB_CENTER_CROSS, 0);
    Pgb *hscale = pgb_grouped(PGB_CENTER_CROSS, 0);

    pgb_add(lscale, pgb_label("L", PGB_CENTER_CROSS));
    pgb_add(cscale, pgb_label("C", PGB_CENTER_CROSS));
    pgb_add(hscale, pgb_label("H", PGB_CENTER_CROSS));
    for (int i = 0; i < 11; i++) {
        pgb_add(lscale, new_swatch(SWATCHX, SWATCHY, box, pick->lscale + i, 0));
        pgb_add(cscale, new_swatch(SWATCHX, SWATCHY, box, pick->cscale + i, 0));
        pgb_add(hscale, new_swatch(SWATCHX, SWATCHY, box, pick->hscale + i, 0));
    }

    pgb_add_list(box, (Pgb*[]){
        pgb_header("Color Picker", PGB_FILL_CROSS),

        pgb_pad(),

        pgb_grouped(PGB_FILL_CROSS | PGB_PACK_HORIZ, (Pgb*[]) {
            pgb_fill(),

            // Detail square.
            pgb_grouped(PGB_CENTER_CROSS, (Pgb*[]){
                pgb_hgrouped(0, (Pgb*[]) {
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 0, 0),
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 1, 0),
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 2, 0),
                    0 }),
                pgb_hgrouped(0, (Pgb*[]) {
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 3, 0),
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 4, 0),
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 5, 0),
                    0 }),
                pgb_hgrouped(0, (Pgb*[]) {
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 6, 0),
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 7, 0),
                    new_swatch(SWATCHX, SWATCHY, box, pick->variants + 8, 0),
                    0 }),
                0 }),

            pgb_pad(),
            lscale,
            cscale,
            hscale,
            pgb_fill(),
            0
        }),

        pgb_pad(),

        new_swatch(32.0f, 64.0f, box, &pick->color, PGB_FILL_CROSS),

        pgb_pad(),

        // Footer.
        pgb_grouped(PGB_CENTER_CROSS, (Pgb*[]) {
            pgb_grouped(0, (Pgb*[]) {
                pgb_grouped(PGB_PACK_HORIZ | PGB_FILL_CROSS, (Pgb*[]) {
                    pgb_fill(),
                    pgb_label("LCHab", PGB_CENTER_CROSS),
                    pick->output = pgb_textbox("", 0, PGB_CENTER_CROSS),
                    0 }),
                pgb_grouped(PGB_PACK_HORIZ | PGB_FILL_CROSS, (Pgb*[]) {
                    pgb_fill(),
                    pgb_label("RGB", PGB_CENTER_CROSS),
                    pick->rgb = pgb_textbox("", 0, PGB_CENTER_CROSS),
                    0 }),
                0 }),
            pgb_pad(),
            pick->ok = pgb_button("OK", ok, PGB_FILL_CROSS),
            0 }),
        0,
    });

    pgb_set_user(pick->ok, box);
    pop_swatches(box);
}


static
void
_free(Pgb *box)
{
    pgb_defaults().free(box); // Delete all children.
    free(box->sys);
}


static
PgbFn*
methods(void) {
    static bool done;
    static PgbFn methods = {
        .free=_free,
    };

    if (done) return &methods;

    methods = pgb_merge(pgb_defaults(), methods);
    done = true;
    return &methods;
}


Pgb*
pgb_color_picker(void (*done)(Pgb *box, PgColorSpace cspace, PgColor color),
                 PgbFlags flags)
{
    Pgb *box = pgb_box(0, 0, flags, methods());
    box->sys = new(Pick,
                   .cspace = PG_LCHAB,
                   .color = (PgColor) { 0.5f, 0.5f, 0.1f, 1.0f },
                   .done = done);
    setup(box);
    return box;
}
