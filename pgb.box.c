#include <stdlib.h>
#include <math.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>

static void opad(Pgb *box, bool vert, float *ax, float *ay, float *bx, float *by);
static void expand_fills(Pgb *from, float total, int nfills, bool vert);

/*
    Calculate the ideal size of the content of a box.

    The content size is calculated by getting the content
    size and padding of all of its children and adding them
    up along the primary axis and taking the max along the
    cross axis.

    Boxes with fixed positions are not counted.

    Boxes with "fill axis" are counted with their given
    content sizes. They may expand when actually packed.

    With no packing, the size is calculated so that all of
    all of the children are visible including padding.

    Sets box.cx and box.cy.
*/
void
pgb_default_autosize(Pgb *box)
{
    if (!box) return;

    /*
        Don't attempt to size a box with no children.
        Either its size was already set at creation or
        its size will always be set to (0, 0).
     */
    if (!box->child)
        return;

    bool    vert = (box->flags & PGB_PACK) == PGB_PACK_VERT;
    bool    no_pack = (box->flags & PGB_PACK) == PGB_PACK_NONE;
    bool    no_ipad = box->flags & PGB_NO_IPAD;
    float   cx = 0.0f;
    float   cy = 0.0f;

    if (no_pack) {
        /*
            Get the size required to contain all children.
         */
        for (Pgb *c = box->child; c; c = c->next) {
            float ax, ay, bx, by;
            opad(c, vert, &ax, &ay, &bx, &by);
            pgb_autosize(c);
            cx = fmaxf(cx, c->x + c->cx + ax + bx),
            cy = fmaxf(cy, c->y + c->cy + ay + by);
        }

        box->cx = ceilf(cx);
        box->cy = ceilf(cy);
        return;
    }

    for (Pgb *c = box->child; c; c = c->next) {
        /*
            Add the content sizes along the primary axis and
            the max on the cross axis.
         */

        pgb_autosize(c);

        if (c->flags & PGB_FIXED_POS)
            continue;

        float       ipad = no_ipad || !c->next? 0.0f: PAD;
        float       ax, ay, bx, by;
        opad(c, vert, &ax, &ay, &bx, &by);
        float       endx = c->cx + ax + bx;
        float       endy = c->cy + ay + by;

        if (vert)
            cx = fmaxf(cx, endx),
            cy += endy + ipad;
        else
            cx += endx + ipad,
            cy = fmaxf(cy, endy);
    }

    box->cx = ceilf(cx);
    box->cy = ceilf(cy);
}


/*
    Pack box into available space.
    This sets box.sx and box.sy and the
    locations and sizes of all of its children.

    Sets box.sx and box.sy.
 */
void
pgb_default_pack(Pgb *box, float availx, float availy)
{
    if (!box) return;

    PgbFlags    bf = box->flags;
    bool        vert = (bf & PGB_PACK) == PGB_PACK_VERT;
    bool        no_pack = (bf & PGB_PACK) == PGB_PACK_NONE;
    bool        no_ipad = bf & PGB_NO_IPAD;
    bool        wrap = bf & PGB_WRAP;
    float       bax, bay, bbx, bby;
    opad(box, vert, &bax, &bay, &bbx, &bby);


    /*
        If we aren't packing, don't move anything.
        Just set their sizes to the requested size
        and padding.
     */
    if (no_pack) {
        for (Pgb *c = box->child; c; c = c->next) {
            float       ax, ay, bx, by;
            opad(c, vert, &ax, &ay, &bx, &by);

            pgb_autosize(c);
            c->sx = c->cx + ax + bx;
            c->sy = c->cy + ay + by;
            pgb_pack(c, c->sx, c->sy);
        }

        return;
    }


    /*
        Subtract padding from available space.
     */
    availx -= bax + bbx;
    availy -= bay + bby;
    float x = bax;
    float y = bay;
    float *axis = vert? &y: &x;
    float *cross = vert? &x: &y;
    float axis_start = *axis;
    float axis_avail = vert? availy: availx;
    float cross_avail = vert? availx: availy;


    /*
        Pack all of the children in the available space.
     */
    int     nfills = 0;
    Pgb     *base = box->child;
    float   cross_max = *cross;
    for (Pgb *c = box->child; c; c = c->next) {
        float   ax, ay, bx, by;
        opad(c, vert, &ax, &ay, &bx, &by);

        pgb_autosize(c);

        if (c->flags & PGB_FIXED_POS) {
            c->sx = c->cx + ax + bx;
            c->sy = c->cy + ay + by;
            continue;
        }

        int     cross_type = c->flags & PGB_CROSS;
        float   *axis_pos = vert? &c->y: &c->x;
        float   *axis_size = vert? &c->sy: &c->sx;
        float   *cross_size = vert? &c->sx: &c->sy;
        float   *cross_pos = vert? &c->x: &c->y;
        float   ipad = no_ipad || !c->next? 0.0f: PAD;

        c->x = x;
        c->y = y;
        c->sx = c->cx + ax + bx;
        c->sy = c->cy + ay + by;

        if (wrap && *axis_pos + *axis_size > axis_avail) {
            /*
                If overflowed major axis, expand current fills,
                and wrap around.

                It is possible that the box still will not fit.
                That is OK. Just dump it here.
             */
            expand_fills(base, axis_avail - *axis, nfills, vert);
            base = c;
            nfills = 0;
            *axis_pos = axis_start;
            *cross_pos = cross_max + (no_ipad? 0.0f: PAD);
        }

        if (c->flags & PGB_FILL_AXIS)
            nfills++;

        if (cross_type == PGB_FILL_CROSS)
            *cross_size = cross_avail;
        else if (cross_type == PGB_CENTER_CROSS)
            *cross_pos = (cross_avail - *cross_size) * 0.5f;
        else if (cross_type == PGB_END_CROSS)
            *cross_pos = cross_avail - *cross_size;

        *axis += *axis_size + ipad;

        cross_max = fmaxf(cross_max, *cross_pos + *cross_size);

        // Tell child to pack its children in its space.
        pgb_pack(c, c->sx, c->sy);
    }

    expand_fills(base, axis_avail - *axis, nfills, vert);
}


static void
expand_fills(Pgb *from, float total, int nfills, bool vert)
{
    if (!nfills || truncf(total) <= 0.0f)
        return;

    float   extra = total / nfills;
    int     remaining = nfills;

    for (Pgb *c = from; remaining; c = c->next)
        if (c->flags & PGB_FILL_AXIS) {
            *(vert? &c->sy: &c->sx) += extra;
            remaining--;
        }
}


static void
opad(Pgb *box, bool vert, float *ax, float *ay, float *bx, float *by)
{
    PgbFlags f = box->flags;
    if (vert)
        *ax = f & PGB_OPAD_CROSS_START ? PAD: 0.0f,
        *bx = f & PGB_OPAD_CROSS_END ? PAD: 0.0f,
        *ay = f & PGB_OPAD_AXIS_START ? PAD: 0.0f,
        *by = f & PGB_OPAD_AXIS_END ? PAD: 0.0f;
    else
        *ax = f & PGB_OPAD_AXIS_START ? PAD: 0.0f,
        *bx = f & PGB_OPAD_AXIS_END ? PAD: 0.0f,
        *ay = f & PGB_OPAD_CROSS_START ? PAD: 0.0f,
        *by = f & PGB_OPAD_CROSS_END ? PAD: 0.0f;
}


void
pgb_default_draw(Pgb *box, Pg *g)
{
    for (Pgb *c = box->child; c; c = c->next) {
        Pg *sub = pg_subcanvas(g, c->x, c->y, c->sx, c->sy);
        pg_reset_state(sub);
        pgb_draw(sub, c);
        pg_free(sub);
    }
}


static
void
_free(Pgb *box) {
    for (Pgb *i = box->child, *next; i; i = next) {
        next = i->next;
        pgb_free(i);
    }
}


static
void
click(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) box, (void) x, (void) y, (void) button, (void) mods;
}


static
void
mouse_down(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) box, (void) x, (void) y, (void) button, (void) mods;
}


static
void
mouse_up(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) x, (void) y, (void) mods;
    if (pgb_is_dragged(box) && button == 0) {
        pgb_drop();
        pgb_dirty(box);
    }
}


static
void
mouse_move(Pgb *box, float x, float y, int button, PgMods mods)
{
    (void) button, (void) mods;
    if (pgb_is_dragged(box)) {
        PgPt offset = pgb_get_drag_offset();
        box->x += x - offset.x;
        box->y += y - offset.y;
        pgb_dirty(box);
    }
}


static
void
key_down(Pgb *box, PgKey key, PgMods mods)
{
    // Cancel drag if Escape is pressed.
    if (pgb_is_dragged(box) && key == PG_KEY_ESCAPE && !mods)
        pgb_set_dragged(0);
}


static
void
key_up(Pgb *box, PgKey key, PgMods mods)
{
    (void) box, (void) key, (void) mods;
}


static
void
character(Pgb *box, unsigned codepoint)
{
    (void) box, (void) codepoint;
}


static
void
hover(Pgb *box, bool over)
{
    (void) box, (void) over;
}


static
void
focus(Pgb *box, bool on)
{
    (void) box, (void) on;
}


static
void
drag(Pgb *box, bool dragging)
{
    (void) box, (void) dragging;
}


static
void
drop(Pgb *box)
{
    (void) box;
}


static
void
dirty(Pgb *box)
{
    pgb_dirty(box->parent);
}


static
Pgb*
hit(Pgb *box, float x, float y, PgPt *adjusted)
{
    if (!box)
        return 0;

    Pgb *found_child = 0;

    if (!pgb_point_in(box, x, y))
        /*
            If the point is not in our box, return immediately.
         */
        return 0;

    for (Pgb *c = box->child; c; c = c->next) {
        /*
            Search children back-to-front.
            If we find a hit, we must continue looking to
            see if there is another child in front.
         */
        Pgb *tmp = pgb_hit(c, x - box->x, y - box->y, adjusted);
        if (tmp)
            found_child = tmp;
    }

    if (found_child)
        /*
            If we found a child, it has already set adjusted
            so we can just return.
         */
        return found_child;

    if (adjusted)
        *adjusted = PgPt(x - box->x, y - box->y);
    return box;
}


PgbFn
pgb_defaults(void)
{
    static PgbFn methods = {
        pgb_default_draw,
        _free,
        pgb_default_autosize,
        pgb_default_pack,
        click,
        mouse_down,
        mouse_up,
        mouse_move,
        key_down,
        key_up,
        character,
        hover,
        focus,
        drag,
        drop,
        hit,
        dirty,
    };
    return methods;
}


Pgb*
pgb_box(float cx, float cy, PgbFlags flags, const PgbFn *v)
{
    if (!v) return 0;
    cx = ceilf(cx);
    cy = ceilf(cy);

    Pgb *box = calloc(1, sizeof *box);
    box->v = v;
    box->cx = cx;
    box->cy = cy;
    box->flags = flags;
    return box;
}
