/*

    This box is composed:
        Space
          Button
            Canvas (for drawing checkbox)
            Space
            Label (optional)

    The top space is there since we need to set the user pointer
    on box.

    The boxes keep track of each other through the info stored
    in their user pointers. They should not try to navigate the
    tree structure.

 */

#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

static const float SX = 15.0f;
static const float SY = 15.0f;
static const float LINE = 2.0f;
static const float ROUND = 2.0f;

typedef struct Info Info;
struct Info {
    bool checked;
    bool disabled;
    void (*clicked)(Pgb *box);

    Pgb *top;
    Pgb *button;
    Pgb *label;
    Pgb *checkbox;
};


static
PgbColor
text_color(Pgb *button_box, bool disabled)
{
    return disabled? PGB_MID_COLOR:
           pgb_is_focused(button_box)? PGB_ACCENT_COLOR:
           pgb_is_hovered(button_box)? PGB_ACCENT_COLOR:
           PGB_DARK_COLOR;
}


bool
pgb_is_checkbox_checked(Pgb *box)
{
    if (!box) return false;
    return ((Info*) pgb_get_sys(box))->checked;
}


void
pgb_check_checkbox(Pgb *box, bool checked)
{
    if (!box) return;
    ((Info*) pgb_get_sys(box))->checked = checked;
    pgb_dirty(box);
}


bool
pgb_is_checkbox_enabled(Pgb *box)
{
    if (!box) return false;
    return ((Info*) pgb_get_sys(box))->disabled;
}


Pgb*
pgb_enable_checkbox(Pgb *box, bool enabled)
{
    if (!box) return 0;
    Info *info = pgb_get_sys(box);

    info->disabled = !enabled;
    pgb_set_label_color(info->label, text_color(info->button, !enabled));
    pgb_dirty(box);
    return box;
}


static
void
draw(Pgb *box, Pg *g)
{
    Info *info = pgb_get_user(box);
    bool is_hovered = pgb_is_hovered(info->button);

    PgPaint *line_colour = info->disabled? pgb_color(PGB_MID_COLOR):
                           is_hovered? pgb_color(PGB_ACCENT_COLOR):
                           pgb_color(PGB_DARK_COLOR);
    PgPaint *fill_colour = info->disabled && info->checked? pgb_color(PGB_MID_COLOR):
                           info->disabled? pgb_color(PGB_LIGHT_COLOR):
                           info->checked? pgb_color(PGB_ACCENT_COLOR):
                           pgb_color(PGB_LIGHT_COLOR);

    pg_rounded(g,
               LINE / 2.0f,
               LINE / 2.0f,
               box->sx - LINE,
               box->sy - LINE,
               ROUND,
               ROUND);
    pg_set_stroke(g, line_colour);
    pg_set_fill(g, fill_colour);
    pg_fill_stroke(g);
}

static
void
_clicked(Pgb *box)
{
    Info *info = pgb_get_user(box);

    if (!info->disabled) {
        /*
            By default, we change the value before we call
            the callback. The callback is free to change
            the value back.
         */
        info->checked = !info->checked;

        if (info->clicked)
            info->clicked(info->top);

        pgb_dirty(info->top);
    }
}


static
void
hovered(Pgb *box, bool over)
{
    (void) over;
    Info *info = pgb_get_user(box);
    pgb_set_label_color(info->label, text_color(info->button, info->disabled));
    pgb_dirty(box);
}


static
void
focused(Pgb *box, bool on)
{
    (void) on;
    Info *info = pgb_get_user(box);
    pgb_set_label_color(info->label, text_color(info->button, info->disabled));
    pgb_dirty(box);
}


Pgb*
pgb_checkbox(const char *label,
             bool checked,
             void (*clicked)(Pgb *box),
             PgbFlags flags)
{
    /*

        See note at the top of the file.

     */

    Pgb *top, *button, *canvas, *label_box;

    pgb_add(button = pgb_button(0, _clicked, 0),
        pgb_grouped(PGB_PACK_HORIZ | PGB_FILL_CROSS, (Pgb*[]) {
            canvas = pgb_canvas(SX, SY, draw, PGB_CENTER_CROSS),
            label_box = pgb_label(label, PGB_CENTER_CROSS),
            0 }));
    top = pgb_group(button, flags);

    pgb_set_on_button_hover(button, hovered);
    pgb_set_on_button_focus(button, focused);

    Info *info = new(Info,
                   .checked = checked,
                   .clicked = clicked,
                   .top = top,
                   .button = button,
                   .label = label_box,
                   .checkbox = canvas);
    pgb_set_user(canvas, info);
    pgb_set_user(button, info);
    pgb_set_sys(top, info);
    return top;
}
