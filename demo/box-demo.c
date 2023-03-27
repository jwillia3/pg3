#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>


void
draw_buffet(Pgb *box, Pg *g)
{
    PgFont *font = pgb_font(PGB_DEFAULT_FONT);
    float cx = pg_measure_string(font, "Canvas");
    float cy = pg_font_height(font);
    pg_set_clear(g, pg_web_color_paint("wheat"));
    pg_clear(g);
    pg_string_path(g, font, (box->sx - cx)/2, (box->sy - cy)/2, "Canvas");
    pg_fill(g);
}

void
buffet(Pgb *root, PgbFlags flags)
{
    pgb_add(root, pgb_bordered(flags | PGB_WRAP | PGB_PACK_HORIZ, (Pgb*[]) {
        pgb_grouped(0, (Pgb*[]) {
                    pgb_border(pgb_label("Border", 0), 0),
                    pgb_pad(),
                    pgb_header("Header", 0),
                    pgb_label("Label", 0),
                    pgb_pad(),
                    pgb_textbox("Text Box", 0, 0),
                    pgb_enable_textbox(pgb_textbox("Disabled", 0, 0), false),
                    0}),
        pgb_grouped(0, (Pgb*[]) {
            pgb_button("Button", 0, 0),
            pgb_enable_button(pgb_button("Disabled", 0, 0), false),
            pgb_bordered(0, (Pgb*[]) {
                pgb_checkbox("Checked", true, 0, 0),
                pgb_checkbox("Unchecked", false, 0, 0),
                pgb_enable_checkbox(pgb_checkbox("Disabled", false, 0, 0), false),
                pgb_enable_checkbox(pgb_checkbox("Disabled", true, 0, 0), false),
                pgb_grouped(PGB_PACK_HORIZ, (Pgb*[]) {
                    pgb_checkbox(0, true, 0, 0),
                    pgb_checkbox(0, false, 0, 0),
                    pgb_enable_checkbox(pgb_checkbox(0, true, 0, 0), false),
                    pgb_enable_checkbox(pgb_checkbox(0, false, 0, 0), false),
                    0}),
                0 }),
            0 }),
        pgb_canvas(128, 32, draw_buffet, 0),
        pgb_color_picker(0, 0),
        pgb_grouped(0, (Pgb*[]) {
            pgb_grouped(0, (Pgb*[]) {
                        pgb_label("Vertical", 0),
                        pgb_label("Group", 0),
                        0 }),
            pgb_space(0, 8, 0),
            pgb_grouped(PGB_PACK_HORIZ, (Pgb*[]) {
                        pgb_label("Horizontal", 0),
                        pgb_label("Group", 0),
                        0 }),
            0 }),
        pgb_menu(0, 0, (Pgb*[]) {
            pgb_menu_item("Option 1", 0),
            pgb_menu_item("Option 2", "OPTION_2"),
            pgb_menu_item("Option 3", "OPTION_3"),
            0}),
        pgb_grouped(0, (Pgb*[]) {
            pgb_scrollbar(false, .5, 0, 0),
            pgb_scrollbar(true, .5, 0, 0),
            pgb_scrollable(128, 32,
                           pgb_label("This is a scrollable area.", 0),
                           0),
            0}),
        0 }));
}

void
draw_debug(Pgb *box, Pg *g)
{
    (void) box;
    PgPaint bg = pg_linear(PG_LCHAB, box->x, box->y, box->sx, box->sy);
    pg_add_stop(&bg, 0, .5, .5, 0, 1);
    pg_add_stop(&bg, 1, .5, .5, 1, 1);
    pg_set_clear(g, &bg);
    pg_clear(g);
}
Pgb*
debug(int cx, int cy, PgbFlags flags)
{
    return pgb_canvas(cx, cy, draw_debug, flags);
}

void
_changed(Pgb *box, float value)
{
    (void)box;
    printf("GOT VALUE %g\n", value);
}


int
main(int argc, char **argv)
{
    (void) argc;
    (void) argv;

    setvbuf(stdout, 0, _IONBF, 0);
    Pgb *root = pgb_root(pg_window(0, 0, "Smörgåsbord"));

    // buffet(root, 0);
    buffet(root, PGB_FILL);

    // pgb_add_list(root, (Pgb*[]) {
    //         pgb_scrollbar(false, 0.1f, _changed, 0),
    //         pgb_scrollbar(true, 0.1f, _changed, 0),
    //         0 });
    // pgb_set_focused(root->child);

    // pgb_add(root,
    //     pgb_scrollable(320, 240,
    //         // debug(1024, 768, 0),
    //         pgb_label("This is quite long text, this is a test!", 0),
    //         PGB_FILL));

    for (PgEvent e; pg_wait(&e); ) {
        pgb_std_events(e);
        if (e.type == PG_CLOSE_EVENT)
            break;
        if ((e.type == PG_KEY_DOWN_EVENT) &&
            (e.key.key == 'W' && e.key.mods == PG_MOD_CTRL))
            break;
    }
    return 0;
}
