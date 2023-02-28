/*

    Menus are just a group of buttons.

    Each menu item displays itself.
    The value of a menu item is its user pointer.

    Each menu item passed in has a button created for it.
    This button when clicked will call the selected user callback
    with the user-pointer of the item passed in.

    `pgb_menu_item()` is a helper function create text menu items.

    Border
      Button
        Border
          Item content

 */

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

Pgb*
pgb_menu_item(const char *label, void *value)
{
    if (!label)
        return 0;
    if (!value)
        value = (void*) label;
    return pgb_set_user(pgb_label(label, PGB_FILL), value);
}


static
void
item_clicked(Pgb *item_button)
{
    void (*selected)(Pgb *box, void *value) = pgb_get_user(item_button);
    if (selected) {
        Pgb *border = item_button->child;
        Pgb *original = border->child;
        void *value = pgb_get_user(original);
        selected(item_button->parent, value);
    }
}


static
void
item_hover(Pgb *box, bool over)
{
    pgb_set_border_color(box->child, over? PGB_ACCENT_COLOR: PGB_NO_COLOR);
}


Pgb*
pgb_menu(PgbFlags flags, void (*selected)(Pgb *box, void *value), Pgb **items)
{
    static Pgb *ignore[] = { 0 };

    if (!items)
        items = ignore;

    Pgb *box = pgb_border(0, flags | PGB_NO_IPAD);

    for (Pgb **i = items; *i; i++) {
        Pgb *button, *border;

        button = pgb_add(
            pgb_button(0, item_clicked, PGB_FILL),
            border = pgb_border(*i, PGB_FILL));

        pgb_set_border_color(border, PGB_NO_COLOR);
        pgb_set_on_button_hover(button, item_hover);
        pgb_set_user(button, selected);

        pgb_add(box, button);
    }

    pgb_set_border_color(box, PGB_DARK_COLOR);

    return box;
}
