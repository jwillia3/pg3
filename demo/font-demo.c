#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pg3.h"

float   dpi;
PgFont  *ui_font;

struct {
    int         sel;
    PgFamily    *families;
    int         nfamilies;
    int         weight;
    bool        italic;
    PgFont      *the_font;
    float       line_height;
    float       size;
    char        *example;
} fontmgr;

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
init(void)
{
    fontmgr.weight = 400;
    fontmgr.line_height = 1.3;
    fontmgr.size = 11.5;
    fontmgr.example = lorem_ipsum;
}

void
redraw(Pg *g)
{
    PgPaint sel_fg = pg_solid(PG_LCHAB, 0.125, 0.3, 0.09, 1.0);
    PgPaint fg = pg_solid(PG_LCHAB, 0.125, 0.0, 0.0, 1.0);
    PgPaint bg = pg_solid(PG_LCHAB, 0.95, 0.0, 0.0, 1.0);
    pg_canvas_set_fill(g, &fg);
    pg_canvas_set_clear(g, &bg);

    pg_font_scale(ui_font, 9.0 * dpi / 72.0, 0.0);

    PgFont      *font;
    float       screen_height = pg_canvas_get_size(g).y;
    float       line_height = pg_font_height(ui_font) * 1.25;
    int         fit = floor(screen_height / line_height) - 1;
    int         top = fontmgr.sel >= fit? fontmgr.sel - fit: 0;
    float       max_x = 0;
    float       y = 0;

    pg_canvas_clear(g);

    /*
        Recalculate the font list.
    */
    fontmgr.families = pg_font_list();
    fontmgr.nfamilies = 0;
    while (fontmgr.families[fontmgr.nfamilies].name) fontmgr.nfamilies++;

    /*
        Remake the font if its changed since last time.
    */
    if (!fontmgr.the_font) {
        PgFamily fam = fontmgr.families[fontmgr.sel];
        fontmgr.the_font = pg_font_find(fam.name, fontmgr.weight, fontmgr.italic);
        pg_font_scale(fontmgr.the_font, fontmgr.size * dpi / 72.0, 0.0);
    }

    for (int i = top; i < fontmgr.nfamilies && y < screen_height; i++) {
        float x = pg_canvas_printf(g, ui_font, 0, y, "%s", fontmgr.families[i].name);
        max_x = fmaxf(max_x, x);

        if (i == fontmgr.sel) {
            pg_canvas_set_fill(g, &sel_fg);
            pg_canvas_fill(g);
            pg_canvas_set_fill(g, &fg);
        } else
            pg_canvas_fill(g);

        y += line_height;
    }
    max_x += 8;

    PgFamily    fam = fontmgr.families[fontmgr.sel];
    float       x = max_x;
    float       main_y;
    y = 0;

    pg_font_scale(ui_font, 48.0 * dpi / 72.0, 0.0);
    pg_canvas_printf(g, ui_font, x, y, "%s", fam.name);
    pg_canvas_fill(g);
    y += pg_font_height(ui_font) * 1.25;

    pg_font_scale(ui_font, 12.0 * dpi / 72.0, 0.0);
    pg_canvas_printf(g, ui_font, x, y, "W: Select weight: %d", fontmgr.weight);
    pg_canvas_fill(g);
    y += line_height;
    pg_canvas_printf(g, ui_font, x, y, "I: Italic: %s", fontmgr.italic? "ON": "OFF");
    pg_canvas_fill(g);
    y += line_height;
    pg_canvas_printf(g, ui_font, x, y, "L: Line Height: %g", fontmgr.line_height);
    pg_canvas_fill(g);
    y += line_height;
    pg_canvas_printf(g, ui_font, x, y, "S: Size: %g", fontmgr.size);
    pg_canvas_fill(g);
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
            float   avail = pg_canvas_get_size(g).x - x - 10;
            while (i < len && y < screen_height) {
                int n = pg_font_fit_chars(font, e + i, len - i, avail);
                pg_canvas_trace_chars(g, font, x, y, e + i, n);
                pg_canvas_fill(g);
                i += n;
                y += line_height;
            }
            y += line_height;
            e += len;
            if (*e == '\n') e++;
        }
    }

    pg_window_update(g);
}



void
on_update(PgWindow *win, Pg *g)
{
    redraw(g);
}

void
on_keydown(PgWindow *win, int key, unsigned mod)
{
    key = toupper(key);

    if (key == 'W' - 0x40 || key == 'Q' - 0x40)
        exit(0);
    else if (key == PG_KEY_DOWN) {
        if (++fontmgr.sel >= fontmgr.nfamilies)
            fontmgr.sel = fontmgr.nfamilies - 1;
        goto changed_font;
    }
    else if (key == PG_KEY_UP) {
        if (--fontmgr.sel < 0)
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
    pg_font_free(fontmgr.the_font);
    fontmgr.the_font = 0;
    redraw(win->g);
}
int main(int argc, char **argv)
{
    init();

    PgWindow *win = pg_window_open(1280, 1024, "This demo");

    if (!win)
        return 0;

    dpi = pg_window_get_dpi(win).y;

    win->on_update = on_update;
    win->on_keydown = on_keydown;

    ui_font = pg_font_find("system-ui, any", 400, false);

    if (!ui_font)
        puts("cannot open font"),
        exit(0);

    while (pg_event_wait());
}
