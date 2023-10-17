#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>

float       dpi;
PgFont      *ui_small_font;
PgFont      *ui_med_font;
PgFont      *ui_title_font;
PgWindow    *win;
PgPaint     *sel_bg;
PgPaint     *sel_border;
PgPaint     *fg;
PgPaint     *bg;

struct {
    int         sel;
    int         nfamilies;
    int         weight;
    bool        italic;
    bool        fixed;
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
    fontmgr.size = 10.0;
    fontmgr.example = lorem_ipsum;

    sel_bg = pg_paint_new_solid(PG_LCHAB, 0.25, 1.0, 0.65, 0.25);
    sel_border = pg_paint_new_solid(PG_LCHAB, 0.25, 1.0, 0.65, 1);
    fg = pg_paint_new_solid(PG_LCHAB, 0.125, 0.0, 0.0, 1.0);
    bg = pg_paint_new_solid(PG_LCHAB, 0.95, 0.0, 0.0, 1.0);
}

void
redraw(Pg *g)
{
    pg_canvas_set_fill(g, fg);
    pg_canvas_set_clear(g, bg);

    PgFont      *font;

    float       screen_height = pg_canvas_get_size(g).y;
    float       max_x = 0;
    float       y = 0;

    pg_canvas_clear(g);

    /*
        Recalculate the font list.
    */
    const PgFamily *tmp = pg_font_list();
    PgFamily *families = malloc(sizeof * families * pg_font_list_get_count());
    if (fontmgr.fixed) {
        fontmgr.nfamilies = 0;
        for (int i = 0; tmp[i].name; i++) {
            for (int j = 0; j < tmp[i].nfaces; j++)
                if (tmp[i].faces[j].is_fixed) {
                    families[fontmgr.nfamilies] = tmp[i];
                    fontmgr.nfamilies++;
                    break;
                }
        }
    } else {
        for (int i = 0; tmp[i].name; i++)
            families[i] = tmp[i];
        fontmgr.nfamilies = pg_font_list_get_count();
    }

    /*
        Remake the font if its changed since last time.
    */
    if (!fontmgr.the_font) {
        PgFamily fam = families[fontmgr.sel];
        fontmgr.the_font = pg_font_find(fam.name, fontmgr.weight, fontmgr.italic);
        pg_font_scale(fontmgr.the_font, fontmgr.size * dpi / 72.0, 0.0);
    }

    /*
        Print font list.
    */
    {
        float       h = pg_font_get_height(ui_small_font);
        float       line_height = h * 1.25;
        float       gap = (line_height - h) * .5;
        int         fit = floor(screen_height / line_height) - 1;
        int         top = fontmgr.sel >= fit? fontmgr.sel - fit: 0;

        max_x = 0;
        for (int i = top; i < fontmgr.nfamilies && y < screen_height; i++) {
            float x = pg_font_measure_string(ui_small_font, families[i].name);
            max_x = fmaxf(max_x, x + 8);
        }

        for (int i = top; i < fontmgr.nfamilies && y < screen_height; i++) {
            pg_canvas_printf(g, ui_small_font, 0, y, families[i].name);
            y += line_height;

            if (i == fontmgr.sel) {
                pg_canvas_set_fill(g, sel_bg);
                pg_canvas_set_stroke(g, sel_border);
                pg_canvas_rectangle(g, 0, y - line_height - gap, max_x - 4, line_height);
                pg_canvas_fill_stroke(g);
                pg_canvas_set_fill(g, fg);
            }
        }
    }


    /*
        Print settings and info.
     */
    PgFamily    fam = families[fontmgr.sel];
    float       x = max_x;
    float       main_y;
    y = 0;

    pg_canvas_printf(g, ui_title_font, x, y, "%s", fam.name);
    y += pg_font_get_height(ui_title_font) * 1.25;

    pg_canvas_printf(g, ui_small_font, x, y,
                     "W: Select weight: %d\n"
                     "I: Italic: %s\n"
                     "L: Line Height: %g\n"
                     "S: Size: %g\n"
                     "F: Fixed: %s\n"
                     "\n",
                     fontmgr.weight,
                     fontmgr.italic? "ON": "OFF",
                     fontmgr.line_height,
                     fontmgr.size,
                     fontmgr.fixed? "ON": "OFF");
    y += pg_font_get_height(ui_small_font) * 6.0;

    x = pg_canvas_printf(g, ui_small_font, max_x, y, "Path: %s",
                         pg_font_get_path(fontmgr.the_font));
    y += pg_font_get_height(ui_small_font);
    x = pg_canvas_printf(g, ui_small_font, max_x, y, "Weights: ");
    {
        PgFamily fam = families[fontmgr.sel];
        unsigned last = 0;
        for (unsigned i = 0; i < fam.nfaces; i++)
            if (fam.faces[i].is_italic == fontmgr.italic)
                last = i;
        for (unsigned i = 0; i < fam.nfaces; i++)
            if (fam.faces[i].is_italic == fontmgr.italic)
                x = pg_canvas_printf(g, ui_small_font,
                                     x, y,
                                     "%d-%s%s",
                                     fam.faces[i].weight,
                                     fam.faces[i].style,
                                     last == i? "": ", ");
    }
    y += pg_font_get_height(ui_small_font) * 2;

    main_y = y;

    font = fontmgr.the_font;
    float line_height = pg_font_get_height(font) * fontmgr.line_height;

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

    pg_canvas_commit(g);
    pg_window_update(win);
}

void
on_keydown(PgWindow *win, const char *key)
{
    if (!strcmp("Ctrl+W", key) || !strcmp("Ctrl+Q", key))
        exit(0);
    else if (!strcmp("Down", key)) {
        if (++fontmgr.sel >= fontmgr.nfamilies)
            fontmgr.sel = fontmgr.nfamilies - 1;
        goto changed_font;
    }
    else if (!strcmp("Up", key)) {
        if (--fontmgr.sel < 0)
            fontmgr.sel = 0;
        goto changed_font;
    }
    else if (!strcmp("PageDown", key)) {
        fontmgr.sel += 10;
        if (fontmgr.sel >= fontmgr.nfamilies)
            fontmgr.sel = fontmgr.nfamilies - 1;
        goto changed_font;
    }
    else if (!strcmp("PageUp", key)) {
        fontmgr.sel -= 10;
        if (fontmgr.sel < 0)
            fontmgr.sel = 0;
        goto changed_font;
    }
    else if (!strcmp("F", key)) {
        fontmgr.fixed ^= 1;
        fontmgr.sel = 0;
        goto changed_font;
    }
    else if (!strcmp("I", key)) {
        fontmgr.italic ^= 1;
        goto changed_font;
    }
    else if (!strcmp("W", key)) {
        fontmgr.weight += 100;
        goto changed_font;
    }
    else if (!strcmp("Shift+W", key)) {
        fontmgr.weight -= 100;
        goto changed_font;
    }
    else if (!strcmp("L", key)) {
        fontmgr.line_height += 0.1;
        goto changed_font;
    }
    else if (!strcmp("Shift+L", key)) {
        fontmgr.line_height -= 0.1;
        goto changed_font;
    }
    else if (!strcmp("S", key)) {
        fontmgr.size += 1;
        goto changed_font;
    }
    else if (!strcmp("Shift+S", key)) {
        fontmgr.size -= 1;
        goto changed_font;
    }

    return;

    changed_font:
    pg_font_free(fontmgr.the_font);
    fontmgr.the_font = 0;
    redraw(pg_window_get_canvas(win));
}
int main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    init();

    win = pg_window_open(1280, 1024, "Font Viewer");

    if (!win)
        return 0;

    dpi = pg_window_get_dpi(win).y;

    ui_small_font = pg_font_find("system-ui, any", 400, false);
    ui_med_font = pg_font_find("system-ui, any", 400, false);
    ui_title_font = pg_font_find("system-ui, any", 400, false);

    pg_font_scale(ui_small_font, 9.0 * dpi / 72.0, 0.0);
    pg_font_scale(ui_med_font, 12.0 * dpi / 72.0, 0.0);
    pg_font_scale(ui_title_font, 36.0 * dpi / 72.0, 0.0);

    if (!ui_small_font || !ui_med_font || !ui_title_font)
        puts("cannot open font"),
        exit(0);

    PgWindowEvent   *e;
    while ((e = pg_window_event_wait())) {
        switch (e->type) {
        case PG_EVENT_PAINT:
        case PG_EVENT_RESIZED:
            redraw(pg_window_get_canvas(win));
            break;

        case PG_EVENT_KEY_DOWN:
            on_keydown(e->win, e->key.key);
            break;

        case PG_EVENT_CLOSED:
            return 0;
        default:
            break;
        }
    }
}
