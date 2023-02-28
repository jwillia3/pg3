#include <stdlib.h>

#include <pg3.h>
#include <pgbox.h>

static volatile bool    initialized;
static PgFont           *ui_font;
static PgPaint          ui_dark;
static PgPaint          ui_mid;
static PgPaint          ui_light;
static PgPaint          ui_transparent;
static PgPaint          ui_accent;
static PgFont           *ui_font;
static PgFont           *ui_header_font;
static Pgb              *hovered_control;
static Pgb              *focused_control;
static Pgb              *dragged_control;
static PgPt             drag_offset;


static
void
init(void)
{
    if (initialized) return;

    const char *families = "system-ui, sans-serif, any";

    ui_transparent  = pg_solid(PG_LCHAB, 0.0f, 0.0f, 0.0f, 0.0f);
    ui_dark = pg_solid(PG_LCHAB, 0.1f, 0.0f, 0.0f, 1.0f);
    ui_mid = pg_solid(PG_LCHAB, 0.5f, 0.0f, 0.0f, 1.0f);
    ui_light = pg_solid(PG_LCHAB, 0.9f, 0.0f, 0.0f, 1.0f);
    ui_accent = pg_solid(PG_LCHAB, 0.2f, 0.5f, 0.75f, 1.0f);
    ui_font = pg_find_font(families, 500, false);
    ui_header_font = pg_find_font(families, 600, false);
    pg_scale_font(ui_font, 12.0f * pg_dpi().x / 72.0f, 0.0f);
    pg_scale_font(ui_header_font, 18.0f * pg_dpi().x / 72.0f, 0.0f);

    initialized = true;
}


void*
pgb_get_user(Pgb *box)
{
    return box? box->user: 0;
}


Pgb*
pgb_set_user(Pgb *box, void *ptr)
{
    if (!box) return 0;
    box->user = ptr;
    return box;
}


void*
pgb_get_sys(Pgb *box)
{
    return box? box->sys: 0;
}


Pgb*
pgb_set_sys(Pgb *box, void *ptr)
{
    if (!box) return 0;
    box->sys = ptr;
    return box;
}


PgPaint*
pgb_color(PgbColor colour)
{
    init();

    switch (colour) {
    case PGB_NO_COLOR:
        return &ui_transparent;
    case PGB_LIGHT_COLOR:
        return &ui_light;
    case PGB_MID_COLOR:
        return &ui_mid;
    case PGB_DARK_COLOR:
        return &ui_dark;
    case PGB_ACCENT_COLOR:
        return &ui_accent;
    }

    return 0;
}


PgFont*
pgb_font(PgbFont font)
{
    init();

    switch (font) {
    case PGB_DEFAULT_FONT:
        return ui_font;
    case PGB_HEADER_FONT:
        return ui_header_font;
    }

    return 0;
}


void
pgb_free(Pgb *box)
{
    if (!box)
        return;
    pgb_clear_status(box);
    pgb_on_free(box);
    free(box);
}


void
pgb_dirty(Pgb *box)
{
    pgb_on_dirty(box);
}


void
pgb_draw(Pg *g, Pgb *box)
{
    pgb_on_draw(box, g);
}


void
pgb_autosize(Pgb *box)
{
    pgb_on_autosize(box);
}


void
pgb_pack(Pgb *box, float availx, float availy)
{
    pgb_on_pack(box, availx, availy);
}


void
pgb_set_hovered(Pgb *box)
{
    if (hovered_control != box) {
        Pgb *old = hovered_control;
        hovered_control = 0;
        pgb_on_hover(old, false);
    }
    if (!hovered_control) {
        hovered_control = box;
        pgb_on_hover(hovered_control, true);
    }
}


Pgb*
pgb_get_hovered(void)
{
    return hovered_control;
}


bool
pgb_is_hovered(Pgb *box)
{
    return hovered_control == box;
}


void
pgb_set_focused(Pgb *box)
{
    if (focused_control != box) {
        Pgb *old = focused_control;
        focused_control = 0;
        pgb_on_focus(old, false);
        pgb_dirty(old);
    }
    if (!focused_control) {
        focused_control = box;
        pgb_on_focus(focused_control, true);
        pgb_dirty(focused_control);
    }
}


Pgb*
pgb_get_focused(void)
{
    return focused_control;
}


bool
pgb_is_focused(Pgb *box)
{
    return focused_control == box;
}


void
pgb_set_dragged(Pgb *box)
{
    if (dragged_control && box)
        // Can't set another control as dragged.
        return;

    if (dragged_control && !box) {
        // Stopping drag. Not necessarily dropped
        // (e.g. if use pressed Escape).
        // Call pgb_drop() to drop.
        Pgb *old = dragged_control;
        dragged_control = 0;
        pgb_on_drag(old, false);
        pgb_dirty(old);
        return;
    }

    dragged_control = box;
    dragged_control->flags |= PGB_FIXED_POS;
    pgb_bring_to_front(dragged_control);
    pgb_on_drag(dragged_control, true);
}


Pgb*
pgb_get_dragged(void)
{
    return dragged_control;
}


bool
pgb_is_dragged(Pgb *box)
{
    return dragged_control == box;
}


void
pgb_set_drag_offset(float x, float y)
{
    if (!dragged_control) return;
    drag_offset = PgPt(x, y);
}


PgPt
pgb_get_drag_offset(void)
{
    if (!dragged_control) return PgPt(0, 0);
    return drag_offset;
}


void
pgb_drop(void)
{
    if (!dragged_control)
        return;

    pgb_on_drop(dragged_control);
    pgb_set_dragged(0);
}


void
pgb_clear_status(Pgb *box)
{
    if (pgb_is_focused(box))
        pgb_set_focused(0);
    if (pgb_is_hovered(box))
        pgb_set_hovered(0);
    if (pgb_is_dragged(box))
        pgb_set_dragged(0);
}

PgbFn
pgb_merge(PgbFn old, PgbFn new)
{
    return (PgbFn) {
        new.draw? new.draw: old.draw,
        new.free? new.free: old.free,
        new.autosize? new.autosize: old.autosize,
        new.pack? new.pack: old.pack,
        new.click? new.click: old.click,
        new.mouse_down? new.mouse_down: old.mouse_down,
        new.mouse_up? new.mouse_up: old.mouse_up,
        new.mouse_move? new.mouse_move: old.mouse_move,
        new.key_down? new.key_down: old.key_down,
        new.key_up? new.key_up: old.key_up,
        new.character? new.character: old.character,
        new.hover? new.hover: old.hover,
        new.focus? new.focus: old.focus,
        new.drag? new.drag: old.drag,
        new.drop? new.drop: old.drop,
        new.hit? new.hit: old.hit,
        new.dirty? new.dirty: old.dirty,
    };
}


void
pgb_remove(Pgb *child)
{
    if (!child || !child->parent) return;

    /*
        Remove any special status cautiously.
        Even if these do go back into a visible tree,
        they should not retain their status.
    */
    pgb_clear_status(child);

    Pgb *parent = child->parent;

    Pgb **ptr = &parent->child;

    while (*ptr && *ptr != child)
        ptr = &(*ptr)->next;

    if (*ptr == child) {
        *ptr = child->next;
        child->next = 0;
        child->parent = 0;
    }

    pgb_dirty(parent);
}


Pgb*
pgb_add(Pgb *parent, Pgb *child)
{
    if (!parent)
        return 0;
    if (!child || child->parent == parent)
        return parent;

    pgb_remove(child);

    Pgb **ptr = &parent->child;
    while (*ptr)
        ptr = &(*ptr)->next;

    child->parent = parent;
    child->next = 0;
    *ptr = child;

    pgb_dirty(parent);
    return parent;
}


Pgb*
pgb_add_list(Pgb *parent, Pgb **children)
{
    if (!parent || !children)
        return parent;

    for (Pgb **i = children; *i; i++)
        pgb_add(parent, *i);
    return parent;
}


Pgb*
pgb_addv(Pgb *parent, va_list ap)
{
    if (!parent) return 0;

    for (Pgb *child; (child = va_arg(ap, Pgb*)); )
         pgb_add(parent, child);
    va_end(ap);

    return parent;
}


Pgb*
pgb_add_all(Pgb *parent, ...) {
    va_list ap;
    va_start(ap, parent);
    return pgb_addv(parent, ap);
}


void
pgb_bring_to_front(Pgb *box)
{
    if (!box)
        return;
    if (!box->parent)
        return;

    Pgb **ptr = &box->parent->child;
    while (*ptr)
        if (*ptr == box)
            *ptr = box->next;
        else
            ptr = &(*ptr)->next;

    *ptr = box;
    box->next = 0;

    pgb_dirty(box);
}


bool
pgb_point_in(Pgb *box, float x, float y)
{
    if (!box) return false;

    return box &&
            box->x <= x && x <= box->x + box->sx &&
            box->y <= y && y <= box->y + box->sy;
}


Pgb*
pgb_hit(Pgb *box, float x, float y, PgPt *adjusted)
{
    return pgb_on_hit(box, x, y, adjusted);
}


PgPt
pgb_abs_pos(Pgb *child)
{
    if (!child) return PgPt(0, 0);
    float   x = 0.0f;
    float   y = 0.0f;
    for (Pgb *p = child; p; p = p->parent)
        x += p->x,
        y += p->y;
    return PgPt(x, y);
}


void
pgb_std_events(PgEvent e) {
    Pgb   *root = pg_get_box(e.g);
    PgPt  cursor;
    Pgb   *hit;

    switch (e.type) {

    case PG_REDRAW_EVENT:
        root->sx = e.g->sx;
        root->sy = e.g->sy;
        pgb_pack(root, e.g->sx, e.g->sy);
        pgb_draw(e.g, root);
        break;

    case PG_RESIZE_EVENT:
        pg_resize(e.g, e.resized.sx, e.resized.sy);
        root->sx = e.g->sx;
        root->sy = e.g->sy;
        pgb_pack(root, e.g->sx, e.g->sy);
        pgb_draw(e.g, root);
        break;

    case PG_KEY_DOWN_EVENT:
        hit = pgb_get_focused()? pgb_get_focused():
              pgb_get_dragged()? pgb_get_dragged():
              root;
        pgb_on_key_down(hit, e.key.key, e.key.mods);
        break;

    case PG_KEY_UP_EVENT:
        hit = pgb_get_focused()? pgb_get_focused():
              pgb_get_dragged()? pgb_get_dragged():
              root;
        pgb_on_key_up(hit, e.key.key, e.key.mods);
        break;

    case PG_CHAR_EVENT:
        hit = pgb_get_focused()? pgb_get_focused():
              pgb_get_dragged()? pgb_get_dragged():
              root;
        pgb_on_character(hit, e.codepoint);
        break;

    case PG_MOUSE_MOVE_EVENT:
        if ((hit = pgb_get_dragged())) {
            cursor = pgb_abs_pos(hit);
            cursor.x = e.mouse.x - cursor.x;
            cursor.y = e.mouse.y - cursor.y;
        }

        if (hit || (hit = pgb_hit(root, e.mouse.x, e.mouse.y, &cursor))) {
            pgb_set_hovered(hit);
            pgb_on_mouse_move(hit, cursor.x, cursor.y,
                              e.mouse.button, e.mouse.mods);
        }
        break;

    case PG_MOUSE_DOWN_EVENT:
        if ((hit = pgb_get_dragged())) {
            cursor = pgb_abs_pos(hit);
            cursor.x = e.mouse.x - cursor.x;
            cursor.y = e.mouse.y - cursor.y;
        }

        if (hit || (hit = pgb_hit(root, e.mouse.x, e.mouse.y, &cursor)))
            pgb_on_mouse_down(hit, cursor.x, cursor.y,
                             e.mouse.button, e.mouse.mods);
        break;

    case PG_MOUSE_UP_EVENT:
        if ((hit = pgb_get_dragged())) {
            cursor = pgb_abs_pos(hit);
            cursor.x = e.mouse.x - cursor.x;
            cursor.y = e.mouse.y - cursor.y;
        }

        if (hit || (hit = pgb_hit(root, e.mouse.x, e.mouse.y, &cursor))) {
            pgb_on_mouse_up(hit, cursor.x, cursor.y,
                           e.mouse.button, e.mouse.mods);
        }
        break;

    default:
        break;
    }
}


void
pgb_on_draw(Pgb *box, Pg *g)
{
    if (box && box->v->draw) box->v->draw(box, g);
}


void
pgb_on_free(Pgb *box)
{
    if (box && box->v->free) box->v->free(box);
}


void
pgb_on_autosize(Pgb *box)
{
    if (box && box->v->autosize) box->v->autosize(box);
}


void
pgb_on_pack(Pgb *box, float availx, float availy)
{
    if (box && box->v->pack) box->v->pack(box, availx, availy);
}


void
pgb_on_click(Pgb *box, float x, float y, int button, PgMods mods)
{
    if (box && box->v->click) box->v->click(box, x, y, button, mods);
}


void
pgb_on_mouse_down(Pgb *box, float x, float y, int button, PgMods mods)
{
    if (box && box->v->mouse_down) box->v->mouse_down(box, x, y, button, mods);
}


void
pgb_on_mouse_up(Pgb *box, float x, float y, int button, PgMods mods)
{
    if (box && box->v->mouse_up) box->v->mouse_up(box, x, y, button, mods);
}


void
pgb_on_mouse_move(Pgb *box, float x, float y, int button, PgMods mods)
{
    if (box && box->v->mouse_move) box->v->mouse_move(box, x, y, button, mods);
}


void
pgb_on_key_down(Pgb *box, PgKey key, PgMods mods)
{
    if (box && box->v->key_down) box->v->key_down(box, key, mods);
}


void
pgb_on_key_up(Pgb *box, PgKey key, PgMods mods)
{
    if (box && box->v->key_up) box->v->key_up(box, key, mods);
}


void
pgb_on_character(Pgb *box, unsigned codepoint)
{
    if (box && box->v->character) box->v->character(box, codepoint);
}


void
pgb_on_hover(Pgb *box, bool over)
{
    if (box && box->v->hover) box->v->hover(box, over);
}


void
pgb_on_focus(Pgb *box, bool on)
{
    if (box && box->v->focus) box->v->focus(box, on);
}


void
pgb_on_drag(Pgb *box, bool dragging)
{
    if (box && box->v->drag) box->v->drag(box, dragging);
}


void
pgb_on_drop(Pgb *box)
{
    if (box && box->v->drop) box->v->drop(box);
}


void
pgb_on_dirty(Pgb *box)
{
    if (box && box->v->dirty) box->v->dirty(box);
}


Pgb*
pgb_on_hit(Pgb *box, float x, float y, PgPt *adjusted)
{
    return box && box->v->hit? box->v->hit(box, x, y, adjusted): 0;
}
