#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3.h>

Pg      *g;
PgFont  *ui_font;
unsigned demo;

void redraw(void);
void intro_demo(void);
void intro_key(int key, int mod);
void chicago_demo(void);
void fontmgr_init(void);
void fontmgr_demo(void);
void fontmgr_key(int key, int mod);
void oklab_demo(void);
void oklab_key(int key, int mod);

struct demo {
    char *desc;
    void (*init)(void);
    void (*draw)(void);
    void (*key)(int key, int mod);
}
demos[] = {
    {
        .desc="Intro",
        .draw=intro_demo,
        .key=intro_key,
    },
    {
        .desc="Draw Chicago Six-Point Stars",
        .draw=chicago_demo,
        .key=intro_key,
    },
    {
        .desc="Font explorer",
        .init=fontmgr_init,
        .draw=fontmgr_demo,
        .key=fontmgr_key,
    },
    {
        .desc="Oklab Colour Space Comparison",
        .draw=oklab_demo,
        .key=oklab_key,
    }
};
#define NDEMOS (int) (sizeof demos / sizeof *demos)

char lorem_ipsum[] =
        "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Suspendisse"
        " hendrerit, orci et ultricies mattis, erat nisi dignissim felis, sit"
        " amet aliquam nulla felis sed nibh. Proin ac elit non urna fringilla"
        " varius quis et libero. Nullam venenatis et turpis eu ullamcorper."
        " Nunc aliquet ante in dapibus aliquam. Cras sed euismod felis. Proin"
        " non commodo tellus. Duis in quam a dui ultrices molestie eu id"
        " mauris. Donec sed tempor purus, sed tempus massa. Proin leo sapien,"
        " mollis sed lectus eu, viverra fermentum odio. Nunc a elementum ipsum."
        " Vestibulum nec augue enim. Aenean volutpat dui eget mauris egestas,"
        " id tincidunt turpis blandit. Curabitur at pulvinar orci. Pellentesque"
        " habitant morbi tristique senectus et netus et malesuada fames ac"
        " turpis egestas. Sed ullamcorper ipsum ipsum. Integer molestie orci"
        " vel tristique tincidunt.\n"
        "Nam rhoncus enim a est imperdiet tempor. Mauris vehicula ullamcorper"
        " tellus. Cras vel felis turpis. Aenean nisi turpis, imperdiet"
        " vestibulum eleifend vitae, consequat sagittis mi. Duis iaculis"
        " mauris ligula, vitae convallis velit accumsan vitae. Nunc et urna"
        " viverra, luctus dolor eget, mattis ex. Duis eu tempus ipsum, sit"
        " amet auctor lectus. Curabitur bibendum semper magna vitae tempor."
        " In cursus dignissim nunc ac semper. Mauris at mauris eget neque"
        " lacinia sagittis nec vel purus. Vivamus elementum velit eros, non"
        " vulputate metus fermentum eu. Fusce semper augue eu velit venenatis,"
        " sit amet aliquam urna tincidunt. Vestibulum ante ipsum primis in"
        " faucibus orci luctus et ultrices posuere cubilia curae.\n"
        "Class aptent taciti sociosqu ad litora torquent per conubia nostra,"
        " per inceptos himenaeos. Pellentesque in pulvinar risus. Donec auctor"
        " luctus purus in porta. Nullam dui enim, ullamcorper quis ornare at,"
        " condimentum at ante. Suspendisse augue leo, aliquet et congue ut,"
        " sollicitudin non augue. Duis cursus erat id semper mollis."
        " Suspendisse feugiat id odio in vehicula. Aliquam metus massa, ultrices"
        " ut nibh at, hendrerit lacinia nisi. Duis sagittis ante tortor, sit amet"
        " ultricies nisl lobortis ornare. Maecenas ac accumsan felis, id blandit"
        " eros. Fusce tincidunt ullamcorper nunc, sit amet volutpat lorem"
        " eleifend ac. In hac habitasse platea dictumst.\n"
        "In commodo ligula ac velit pulvinar, sed blandit nisi fringilla."
        " Aenean at nulla eu dolor laoreet convallis. Sed ut justo nisl. Nulla"
        " viverra mauris ac leo efficitur feugiat. Vivamus nec quam auctor neque"
        " porta condimentum. Proin sit amet ligula at enim auctor maximus sed ut"
        " ante. Fusce eu posuere quam. Nam fermentum ante non turpis mattis, ac"
        " blandit nisl eleifend. Maecenas nibh libero, sollicitudin sed"
        " hendrerit id, commodo id lorem.\n"
        "Maecenas efficitur hendrerit metus, eget fringilla elit condimentum"
        " non. Aenean bibendum vestibulum turpis, non laoreet neque sodales"
        " eget. Pellentesque nec quam nunc. Donec imperdiet leo mi, eget"
        " malesuada nibh interdum eu. Nunc ut tortor sed neque ultricies"
        " imperdiet. Nam at magna ut lacus condimentum luctus sed sit amet"
        " purus. Mauris sed consequat risus, vestibulum scelerisque lacus."
        " Donec egestas lacus velit, eget laoreet erat imperdiet vel. Mauris"
        " non ipsum metus. Suspendisse ultricies ornare consectetur. Maecenas"
        " eleifend, est in dapibus pellentesque, lorem est sagittis risus,"
        " interdum suscipit odio velit ut felis. Maecenas eget sem quis nunc"
        " ultrices placerat.\n";

void
star(PgPt p, float size, float points)
{
    float f = size; // full size
    float h = size * 0.5f; // half size
    for (float i = 0; i <= points; i++) {
        float tf = 2.0f * 3.14159f * i / points - 3.14159f / 2.0f;
        float th = 2.0f * 3.14159f * (i + 0.5f) / points - 3.14159f / 2.0f;
        if (i == points)
            pg_close(g);
        else {
            if (i == 0)
                pg_move(g, p.x + cosf(tf) * f, p.y + sinf(tf) * f);
            else
                pg_line(g, p.x + cosf(tf) * f, p.y + sinf(tf) * f);
            pg_line(g, p.x + cosf(th) * h, p.y + sinf(th) * h);
        }
    }
}

void
intro_demo(void)
{
    PgPaint fg = pg_solid(PG_LCHAB, 0.125f, 0.0, 0.0, 1.0);
    pg_set_fill(g, &fg);

    pg_printf(g, ui_font, 0, 0, "Select demo with TAB");
    pg_fill(g);

    float line_height = pg_font_height(ui_font) * 1.1;
    float y = line_height;
    for (int i = 0; i < NDEMOS; i++, y += line_height) {
        pg_printf(g, ui_font, 0, y, "%d: %s", i + 1, demos[i].desc);
        pg_fill(g);
    }

    pg_update(g);
}

void
intro_key(int key, int mod)
{
    (void) mod;
    if (key >= '1' && key <= '9') {
        if (key - '1' < NDEMOS)
            demo = key - '1';
        redraw();
    }
}

void
chicago_demo(void)
{
    PgPaint fill = pg_solid(PG_LCHAB, 0.125f, 0.3f, 0.09f, 1.0f);
    PgPaint stroke = pg_solid(PG_LCHAB, 0.125f, 0.3f, 0.65f, 1.0f);
    pg_save(g);
    pg_set_fill(g, &fill);
    pg_set_stroke(g, &stroke);
    pg_set_line_width(g, 10.0f);
    pg_translate(g, pg_size(g).x / 2.0f - 300.0f, pg_size(g).y / 2.0f - 25.0f);
    star(PgPt(000, 0), 50, 6);
    star(PgPt(200, 0), 50, 6);
    star(PgPt(400, 0), 50, 6);
    star(PgPt(600, 0), 50, 6);
    pg_fill_stroke(g);
    pg_restore(g);
    pg_update(g);
}

struct {
    unsigned    sel;
    PgFamily    *families;
    unsigned    nfamilies;
    unsigned    weight;
    bool        italic;
    bool        only_fixed;
    PgFont      *the_font;
    float       line_height;
    float       size;
    char        *example;
} fontmgr;

void
fontmgr_init(void)
{
    fontmgr.weight = 400;
    fontmgr.line_height = 1.3;
    fontmgr.size = 11.5;
    fontmgr.example = lorem_ipsum;
}

bool
is_fixed(PgFamily *family) {
    for (PgFace *f = family->faces; f->family; f++)
        if (f->is_fixed) return true;
    return false;
}

void
fontmgr_demo(void)
{
    PgPaint sel_fg = pg_solid(PG_LCHAB, 0.125, 0.3, 0.09, 1.0);
    PgPaint fg = pg_solid(PG_LCHAB, 0.125, 0.0, 0.0, 1.0);
    PgPaint bg = pg_solid(PG_LCHAB, 0.95, 0.0, 0.0, 1.0);
    pg_set_fill(g, &fg);
    pg_set_clear(g, &bg);

    PgFont      *font;
    float       dpi = pg_dpi().y;
    float       screen_height = pg_size(g).y;
    float       line_height = pg_font_height(ui_font) * 1.25;
    unsigned    fit = floor(screen_height / line_height) - 1;
    unsigned    top = fontmgr.sel >= fit? fontmgr.sel - fit: 0;
    float       max_x = 0;
    float       y = 0;

    pg_clear(g);

    /*
        Recalculate the font list.
    */
    PgFamily *new_set = calloc(pg_get_family_count() + 1, sizeof *new_set);

    fontmgr.nfamilies = 0;
    for (PgFamily *f = pg_list_fonts(), *o = new_set; f->name; f++)
        if (!fontmgr.only_fixed || is_fixed(f))
            *o++ = *f,
            fontmgr.nfamilies++;

    free(fontmgr.families);
    fontmgr.families = new_set;


    /*
        Remake the font if its changed since last time.
    */
    if (!fontmgr.the_font) {
        PgFamily fam = fontmgr.families[fontmgr.sel];
        fontmgr.the_font = pg_find_font(fam.name, fontmgr.weight, fontmgr.italic);
        pg_scale_font(fontmgr.the_font, fontmgr.size * dpi / 72.0, 0.0);
    }

    for (unsigned i = top; i < fontmgr.nfamilies && y < screen_height; i++) {

        bool is_fixed = false;
        for (PgFace *face = fontmgr.families[i].faces;
             !is_fixed && face->family;
             face++)
        {
            is_fixed |= face->is_fixed;
        }

        pg_printf(g, ui_font, 0, y, "%s", fontmgr.families[i].name);

        if (i == fontmgr.sel) {
            pg_set_fill(g, &sel_fg);
            pg_fill(g);
            pg_set_fill(g, &fg);
        } else
            pg_fill(g);

        y += line_height;
    }

    for (unsigned i = 0; i < fontmgr.nfamilies; i++)
        max_x = fmaxf(max_x, pg_measure_string(ui_font, fontmgr.families[i].name));
    max_x += 8;

    PgFamily    fam = fontmgr.families[fontmgr.sel];
    float       x = max_x;
    float       main_y;
    y = 0;

    pg_scale_font(ui_font, 48.0 * dpi / 72.0, 0.0);
    pg_printf(g, ui_font, x, y, "%s", fam.name);
    pg_fill(g);
    y += pg_font_height(ui_font) * 1.25;

    pg_scale_font(ui_font, 12.0 * dpi / 72.0, 0.0);
    pg_printf(g, ui_font, x, y, "W: Select weight: %d", fontmgr.weight);
    pg_fill(g);
    y += line_height;
    pg_printf(g, ui_font, x, y, "I: Italic: %s", fontmgr.italic? "ON": "OFF");
    pg_fill(g);
    y += line_height;
    pg_printf(g, ui_font, x, y, "L: Line Height: %g", fontmgr.line_height);
    pg_fill(g);
    y += line_height;
    pg_printf(g, ui_font, x, y, "S: Size: %g", fontmgr.size);
    pg_fill(g);
    y += line_height;
    pg_printf(g, ui_font, x, y, "F: Spacing: %s", fontmgr.only_fixed? "Fixed-Pitched": "Any");
    pg_fill(g);
    y += line_height;
    y += line_height;
    main_y = y;

    font = fontmgr.the_font;
    line_height = pg_font_height(font) * fontmgr.line_height;

    x = max_x, y = main_y;
    {
        char    *e = fontmgr.example;
        while (*e && y < screen_height) {
            int     i = 0;
            int     len = strcspn(e, "\n");
            float   avail = pg_size(g).x - x - 10;
            while (i < len && y < screen_height) {
                pg_set_text_pos(g, PG_TEXT_POS_TOP);

                int n = pg_fit_chars(font, e + i, len - i, avail);
                pg_chars_path(g, font, x, y, e + i, n);
                pg_fill(g);
                i += n;
                y += line_height;
            }
            y += line_height;
            e += len;
            if (*e == '\n') e++;
        }
    }

    pg_update(g);
}

void
fontmgr_key(int key, int mod)
{
    if (key == PG_KEY_DOWN) {
        if (++fontmgr.sel >= fontmgr.nfamilies)
            fontmgr.sel = fontmgr.nfamilies - 1;
        goto changed_font;
    }
    else if (key == PG_KEY_UP) {
        if (fontmgr.sel != 0)
            fontmgr.sel = fontmgr.sel - 1;
        goto changed_font;
    }
    else if (key == 'F') {
        fontmgr.only_fixed ^= 1;
        fontmgr.sel = 0;
        goto changed_font;
    }
    else if (key == 'I') {
        fontmgr.italic ^= 1;
        goto changed_font;
    }
    else if (key == 'W') {
        fontmgr.weight += 100 * (mod & PG_MOD_SHIFT ? -1: 1);
        goto changed_font;
    }
    else if (key == 'L') {
        fontmgr.line_height += 0.1 * (mod & PG_MOD_SHIFT ? -1: 1);
        goto changed_font;
    }
    else if (key == 'S') {
        fontmgr.size += 1 * (mod & PG_MOD_SHIFT ? -1: 1);
        goto changed_font;
    }

    return;

    changed_font:
    pg_free_font(fontmgr.the_font);
    fontmgr.the_font = 0;
    redraw();
}

struct {
    float lightness;
    float chroma;
    float hue;
} oklab = {
    .lightness = .5,
    .chroma = .1,
    .hue = 0.,
};

void
oklab_key(int key, int mod)
{
    if (key == 'L')
        oklab.lightness += .05 * (mod & PG_MOD_SHIFT ? -1: 1);
    else if (key == 'C')
        oklab.chroma += .01 * (mod & PG_MOD_SHIFT ? -1: 1);
    else if (key == 'H')
        oklab.hue += .05 * (mod & PG_MOD_SHIFT ? -1: 1);
    redraw();
    return;
}

void
oklab_demo(void)
{
    PgPt    sz = pg_size(g);

    PgPaint colour = pg_linear(PG_OKLCH, 0., 0., sz.x, 0.);
    pg_add_stop(&colour, 0., oklab.lightness, oklab.chroma, 0., 1.);
    pg_add_stop(&colour, 1., oklab.lightness, oklab.chroma, 1., 1.);
    pg_set_fill(g, &colour);
    pg_rectangle(g, 0., 0., sz.x, sz.y);
    pg_fill(g);

    pg_set_fill(g, pg_web_color_paint("black"));
    pg_printf(g, ui_font, 0, 0, "LCH: (%g, %g, x)",
              oklab.lightness, oklab.chroma);
    pg_fill(g);

    pg_update(g);
}

void
redraw(void)
{
    PgPaint     clear = pg_solid(PG_LCHAB, 1.0f, 0.0f, 0.0f, 1.0f);

    pg_set_clear(g, &clear);
    pg_clear(g);

    demos[demo % NDEMOS].draw();
}

int main(int argc, char **argv) {
    (void) argc, (void) argv;

    setvbuf(stdout, 0, _IONBF, 0);

    for (int i = 0; i < NDEMOS; i++)
        if (demos[i].init) demos[i].init();

    g = pg_window(0, 0, "Demo");

    const char *font_family = "system-ui, sans-serif, any";
    float font_pts = 12.0;
    ui_font = pg_find_font(font_family, 500, false);
    pg_scale_font(ui_font, font_pts * pg_dpi().x / 72.0, 0.0);

    for (PgEvent e; pg_wait(&e); )
        switch (e.type) {

        case PG_REDRAW_EVENT:
            redraw();
            break;

        case PG_CLOSE_EVENT:
            goto end;

        case PG_KEY_DOWN_EVENT:
            if (e.key.mods & PG_MOD_CTRL && e.key.key == 'W')
                goto end;
            else if (~e.key.mods & PG_MOD_SHIFT && e.key.key == PG_KEY_TAB) {
                demo = (demo + 1) % NDEMOS;
                redraw();
            } else if (e.key.mods & PG_MOD_SHIFT && e.key.key == PG_KEY_TAB) {
                demo = (demo - 1) % NDEMOS;
                redraw();
            }
            else if (demos[demo].key)
                demos[demo].key(e.key.key, e.key.mods);
            break;

        default:
            break;
        }

    end:
    return 0;
}
