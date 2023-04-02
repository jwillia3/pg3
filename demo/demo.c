#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-box.h>





int main()
{
    PgBox *root = pg_new_box_root(pg_window_open(1280, 720, "Demo"), 0);

PgBox *content;
PgBox *commit;

    pg_box_add(root,
        pg_box_vbordered(PG_BOX_FLAG_FILL, (PgBox*[]) {
            pg_box_vgrouped(PG_BOX_FLAG_MINOR_FILL, (PgBox*[]) {
                pg_box_title(PG_BOX_FLAG_MINOR_CENTER, "Resident Evil"),
                pg_box_subtitle(PG_BOX_FLAG_MINOR_CENTER, "Director's Cut"),
                pg_box_heading(PG_BOX_FLAG_MINOR_CENTER, "DualShock Edition"),
                pg_box_text(PG_BOX_FLAG_MINOR_CENTER, "That's a damn long title."),
                NULL}),
            pg_box_bordered(PG_BOX_FLAG_MINOR_FILL,(PgBox*[]) {
                pg_box_fill(),
                pg_box_heading(0, "Header"),
                pg_box_fill(),
                NULL }),
            pg_box_grouped(PG_BOX_FLAG_FILL | PG_BOX_FLAG_NO_PAD, (PgBox*[]) {
                pg_box_vbordered(PG_BOX_FLAG_MINOR_FILL, (PgBox*[]) {
                    pg_box_text(0, "Sidebar"),

                    pg_box_button_text(PG_BOX_FLAG_MINOR_FILL, "Cancel"),
                    pg_box_set_min_size(
                        commit = pg_box_button_text(PG_BOX_FLAG_MINOR_FILL, "Commit"),
                        256,
                        128),
                    NULL }),
                content = pg_box_vbordered(PG_BOX_FLAG_FILL, (PgBox*[]) {
                    pg_box_text(0, "Main Content"),
                    pg_box_heading(0, pg_font_prop_string(pg_box_font_text(), PG_FONT_FULL_NAME)),
                    NULL }),
                NULL }),
            pg_box_bordered(PG_BOX_FLAG_MINOR_FILL,(PgBox*[]) {
                pg_box_fill(),
                // pg_box_set_min_size(pg_box_new_border(PG_BOX_FLAG_NO_PAD), 32, 64),
                // pg_box_bordered(PG_BOX_FLAG_MINOR_FILL | PG_BOX_FLAG_NO_PAD, (PgBox*[]){pg_box_pad(),pg_box_pad(),pg_box_pad(), 0}),
                pg_box_text(PG_BOX_FLAG_MINOR_START, "Footer"),
                pg_box_fill(),
                NULL }),
            NULL }));

PgFamily *fam = pg_font_list();
while (fam->name && strcmp(fam->name, pg_font_prop_string(pg_box_font_text(), PG_FONT_FAMILY))) fam++;
for (PgFace *face = fam->faces; face->family; face++) {
    char buf[256];
    int score =
            -abs((int) 5 - (int) face->width)     * 1000
            -abs((int) 400 - (int) face->weight)  * 1
            -abs((int) 0 - (int) face->is_italic) * 101
            + 0;
    snprintf(buf, sizeof buf, "%3d %d %s", face->weight, score, face->style);
    pg_box_add(content, pg_box_text(0, buf));
}

// pg_box_root_set_hovered(root, commit);


    PgWindowEvent *e;
    while (pg_event_handle(root, e = pg_window_event_wait()))
        if (e->any.type == PG_WINDOW_EVENT_KEY_DOWN &&
            !strcmp("Ctrl+W", e->key.key))
        {
            pg_window_close(pg_box_root_get_window(root));
        }

    pg_window_free(pg_box_root_get_window(root));
    pg_box_free(root, true);
    puts("done.");
}
