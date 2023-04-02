#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-box.h>

struct PgBox {
    const PgBoxType *type;
    PgPt        min;
    PgPt        max;
    PgPt        loc;
    PgPt        size;
    PgBoxFlags  flags;
    void        *sys;
    void        *user;
    PgBox       *parent;
    PgBox       *child;
    PgBox       *next;
};


static const char   text_font_family[] = "system-ui, ui-sans-serif, sans-serif, any";
static const char   title_font_family[] = "system-ui, ui-sans-serif, sans-serif, any";
static const float  text_font_size = 10.0f;
static const float  heading_font_size = 13.0f;
static const float  subtitle_font_size = 17.0f;
static const float  title_font_size = 21.0f;


static inline PgPt
flip(PgPt pt, bool is_vert)
{
    return is_vert? pgpt(pt.y, pt.x): pt;
}


PgPt
pg_box_default_measure(const PgBox *box)
{
    if (!box)
        return pgpt(0, 0);

    bool    vert = box->flags & PG_BOX_FLAG_VERT;
    float   pad = ~box->flags & PG_BOX_FLAG_NO_PAD? 8.0f: 0.0f;
    PgPt    size = { pad, 0.0f };

    for (const PgBox *c = box->child; c; c = c->next) {
        PgPt    tmp = flip(pg_box_measure(c), vert);
        size.x += tmp.x + pad;
        size.y = fmaxf(size.y, tmp.y);
    }

    size.y += 2.0f * pad;
    return flip(size, vert);
}


void
pg_box_default_pack(PgBox *box)
{
    if (!box)
        return;

    bool    vert = box->flags & PG_BOX_FLAG_VERT;
    float   pad = ~box->flags & PG_BOX_FLAG_NO_PAD? 8.0f: 0.0f;

    unsigned nfills = 0;
    float   availx = flip(box->size, vert).x - pad;
    float   availy = flip(box->size, vert).y - 2 * pad;
    for (PgBox *c = box->child; c; c = c->next) {
        c->size = pg_box_measure(c);

        if (c->flags & PG_BOX_FLAG_MAJOR_FILL)
            nfills++;

        availx -= flip(c->size, vert).x + pad;
    }

    float   fill = availx > 0.0f && nfills? availx / nfills: 0.0f;
    float   x = pad;
    for (PgBox *c = box->child; c; c = c->next) {
        PgPt        loc = pgpt(x, pad);
        PgPt        size = flip(c->size, vert);
        unsigned    minor = c->flags & PG_BOX_FLAG_MINOR_MASK;

        if (c->flags & PG_BOX_FLAG_MAJOR_FILL)
            size.x += fill;

        if (minor == PG_BOX_FLAG_MINOR_FILL)
            size.y = availy;
        else if (minor == PG_BOX_FLAG_MINOR_CENTER)
            loc.y += .5f * (availy - size.y);
        else if (minor == PG_BOX_FLAG_MINOR_END)
            loc.y = availy - size.y + pad;

        x += size.x + pad;

        c->loc = flip(loc, vert);
        c->size = flip(size, vert);
        pg_box_pack(c);
    }
}


void
pg_box_default_paint(PgBox *box, Pg *g)
{
    for (PgBox *c = box->child; c; c = c->next) {
        Pg  *sub = pg_canvas_new_subcanvas(g, c->loc.x, c->loc.y, c->size.x, c->size.y);
        pg_box_paint(c, sub);
        pg_canvas_free(sub);
    }
}


PgFont*
pg_box_font_text(void)
{
    static PgFont *font;
    if (!font) {
        font = pg_font_find(text_font_family, 400, false);
        pg_font_scale(font, text_font_size * pg_window_get_dpi(NULL).x / 72.0f, 0.0f);
    }
    return font;
}


PgFont*
pg_box_font_title(void)
{
    static PgFont *font;
    if (!font) {
        font = pg_font_find(title_font_family, 600, false);
        pg_font_scale(font, title_font_size * pg_window_get_dpi(NULL).x / 72.0f, 0.0f);
    }

    return font;
}


PgFont*
pg_box_font_subtitle(void)
{
    static PgFont *font;
    if (!font) {
        font = pg_font_clone(pg_box_font_title());
        pg_font_scale(font, subtitle_font_size * pg_window_get_dpi(NULL).x / 72.0f, 0.0f);
    }

    return font;
}


PgFont*
pg_box_font_heading(void)
{
    static PgFont *font;
    if (!font) {
        font = pg_font_clone(pg_box_font_title());
        pg_font_scale(font, heading_font_size * pg_window_get_dpi(NULL).x / 72.0f, 0.0f);
    }

    return font;
}


PgBox*
pg_box_new(const PgBoxType *type, PgBoxFlags flags)
{
    if (!type)
        return NULL;

    return pgnew(PgBox,
                 .type = type,
                 .flags = flags);
}


void
pg_box_free(PgBox *box, bool free_children)
{
    if (!box)
        return;

    if (free_children)
        for (PgBox *c = box->child; c; c = c->next)
            pg_box_free(c, true);

    if (box->type->free)
        box->type->free(box);
    free(box);
}


void
pg_box_add(PgBox *parent, PgBox *child)
{
    if (!parent || !child)
        return;

    pg_box_remove(child);

    PgBox **lastp = &parent->child;
    while (*lastp)
        lastp = &(*lastp)->next;
    *lastp = child;
}


PgBox*
pg_box_added(PgBox *parent, PgBox **children)
{
    for (PgBox **c = children; *c; c++)
        pg_box_add(parent, *c);
    return parent;
}


void
pg_box_remove(PgBox *child)
{
    if (!child || !child->parent)
        return;

    PgBox   **p = &child->parent->child;
    while (*p && *p != child)
        p = &(*p)->next;
    if (*p)
        *p = child->next;

    child->next = NULL;
    child->parent = NULL;
}


PgPt
pg_box_measure(const PgBox *box)
{
    if (!box)
        return pgpt(0, 0);
    PgPt size = box->type->measure? box->type->measure(box):
                                    pg_box_default_measure(box);

    // Apply limitations.
    size = pgpt(fmaxf(size.x, box->min.x),
                fmaxf(size.y, box->min.y));
    size = pgpt(box->max.x? fminf(size.x, box->max.x): size.x,
                box->max.y? fminf(size.y, box->max.y): size.y);
    return size;
}


void
pg_box_pack(PgBox *box)
{
    if (!box)
        return;
    if (box->type->pack)
        box->type->pack(box);
    else
        pg_box_default_pack(box);
}


void
pg_box_paint(PgBox *box, Pg *g)
{
    if (!box)
        return;
    if (box->type->paint)
        box->type->paint(box, g);
    else
        pg_box_default_paint(box, g);
}


PgPt
pg_box_get_min_size(const PgBox *box)
{
    return box? box->min: pgpt(0, 0);
}


PgPt
pg_box_get_size(const PgBox *box)
{
    return box? box->size: pgpt(0, 0);
}


PgPt
pg_box_get_loc(const PgBox *box)
{
    return box? box->loc: pgpt(0, 0);
}


PgBoxFlags
pg_box_get_flags(const PgBox *box)
{
    return box? box->flags: 0;
}


PgBox*
pg_box_get_parent(const PgBox *box)
{
    return box? box->parent: 0;
}


PgBox*
pg_box_get_child(const PgBox *box)
{
    return box? box->child: 0;
}


PgBox*
pg_box_get_next(const PgBox *box)
{
    return box? box->next: 0;
}


void*
pg_box_get_sys(const PgBox *box)
{
    return box? box->sys: NULL;
}


void*
pg_box_get_user(const PgBox *box)
{
    return box? box->user: NULL;
}


const PgBoxType*
pg_box_get_type(const PgBox *box)
{
    return box? box->type: NULL;
}


PgBox*
pg_box_set_min_size(PgBox *box, float x, float y)
{
    if (box)
        box->min = pgpt(x, y);
    return box;
}


PgBox*
pg_box_set_size(PgBox *box, float x, float y)
{
    if (box)
        box->size = pgpt(x, y);
    return box;
}


PgBox*
pg_box_set_loc(PgBox *box, float x, float y)
{
    if (box)
        box->loc = pgpt(x, y);
    return box;
}


PgBox*
pg_box_set_flags(PgBox *box, PgBoxFlags flags)
{
    if (box)
        box->flags = flags;
    return box;
}


PgBox*
pg_box_set_sys(PgBox *box, void *sys)
{
    if (box)
        box->sys = sys;
    return box;
}


PgBox*
pg_box_set_user(PgBox *box, void *user)
{
    if (box)
        box->user = user;
    return box;
}


static bool
within(PgBox *box, float x, float y)
{
    PgPt    loc = pg_box_get_loc(box);
    PgPt    size = pg_box_get_size(box);
    x -= loc.x;
    y -= loc.y;
    return 0 <= x && x <= size.x && 0 <= y && y <= size.y;
}


PgBox*
pg_box_get_from_loc(PgBox *box, float x, float y)
{
    if (!within(box, x, y))
        return NULL;

    PgBox   *target = box;
    PgBox   *tmp;
    PgPt    loc = pg_box_get_loc(box);
    x -= loc.x;
    y -= loc.y;

    for (PgBox *c = box->child; c; c = c->next)
        if ((tmp = pg_box_get_from_loc(c, x, y)))
            target = tmp;

    if (!pg_box_get_type(target)->hovered)
        return NULL;
    return target;
}

bool
pg_event_handle(PgBox *root, PgWindowEvent *e)
{
    switch (e->any.type) {

    case PG_WINDOW_EVENT_PAINT:
        pg_box_paint(root, pg_window_get_canvas(pg_box_root_get_window(root)));
        break;

    case PG_WINDOW_EVENT_MOUSE_MOVED:
        {
            PgBox *target = pg_box_get_from_loc(root, e->mouse.x, e->mouse.y);
            pg_box_root_set_hovered(root, target);
        }
        break;

    case PG_WINDOW_EVENT_MOUSE_DOWN:
        {
            PgBox *target = pg_box_get_from_loc(root, e->mouse.x, e->mouse.y);
            pg_box_root_set_active(root, target);
        }
        break;

    case PG_WINDOW_EVENT_RESIZED:
        {
            Pg  *g = pg_window_get_canvas(pg_box_root_get_window(root));
            pg_canvas_set_size(g, e->resized.width, e->resized.height);
            pg_box_set_size(root, e->resized.width, e->resized.height);
            pg_box_pack(root);
        }
        break;

    case PG_WINDOW_EVENT_KEY_DOWN:

        break;

    case PG_WINDOW_EVENT_CLOSED:
        return false;

    default:
        break;
    }
    return true;
}