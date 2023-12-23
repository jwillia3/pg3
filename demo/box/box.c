#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-box.h>

struct pgb_prop_t {
    char*           name;
    char*           value;
    pgb_prop_t*     next;
};


static PgFont       *font;
static PgFont       *monospace_font;
static PgFont       *subtitle_font;
static PgFont       *title_font;


static void free_props(pgb_prop_t *props);
static void set_prop(pgb_prop_t **props, const char *name, const char *value);
static const char* get_prop(const pgb_prop_t *props, const char *name);


pgb_t*
pgb_box(const pgb_type_t *type, ...)
{
    if (!type)
        return NULL;

    pgb_t *box = pgnew(pgb_t, .type = type);
    pgb_props_set(box,
        "vertical", "false",
        "major", "start",
        "minor", "start",
        "float", "false",
        "ipad", "0",
        "opad", "0",
        "item-spacing", "6",
        "min-x", "0",
        "min-y", "0",
        "max-x", "infinity",
        "max-y", "infinity",
        "can-focus", "false",
        "opaque", "false",
        NULL);

    va_list ap;
    va_start(ap, type);
    pgb_props_set_var(box, va_arg(ap, char*), ap);
    va_end(ap);
    return box;
}


void
pgb_free(pgb_t *box)
{
    if (!box) return;

    if (box->type && box->type->free)
        box->type->free(box);

    for (pgb_handler_t *n, *i = box->handlers; i; i=n)
        n = i->next,
        free((void*) i->event),
        free(i);

    free_props(box->props);
    free(box);
}


static PgPt
flip(PgPt p, bool vert)
{
    return vert? pgpt(p.y, p.x): p;
}


PgPt
pgb_default_measure(const pgb_t *box)
{
    if (!box)
        return pgpt(0, 0);

    /*
        Add up children along the major axis and get the max
        value of any child along the minor axis.
        For simplicity, this is written as if every box is
        horizontal, and the results can be flipped.
    */
    bool    vert = pgb_prop_get_bool(box, "vertical");
    float   ipad = pgb_prop_get_float(box, "ipad");
    float   spacing = pgb_prop_get_float(box, "item-spacing");
    PgPt    caret = pgpt(0, 0);
    for (const pgb_t *c = box->children; c; c = c->next) {
        if (pgb_prop_get_bool(c, "float"))
            continue;
        PgPt    size = flip(pgb_measure(c), vert);
        float   between = c == box->children? 0.0f: spacing;
        float   opad = pgb_prop_get_float(c, "opad");
        float   sx = size.x + opad * 2.0f + between;
        float   sy = size.y + opad * 2.0f;
        caret.x += sx;
        caret.y = fmaxf(caret.y, sy);
    }
    caret = pgpt(caret.x + ipad * 2.0f, caret.y + ipad * 2.0f);
    return pgb_restrict(box, flip(caret, vert));
}


void
pgb_default_pack(pgb_t *box)
{
    if (!box) return;

    /*
        Assign children positions and sizes along the major axis.
        For simplicity, this is written as if every box is
        horizontal, and the results can be flipped.
    */
    bool        vert = pgb_prop_get_bool(box, "vertical");
    unsigned    nfills = 0;
    float       ipad = pgb_prop_get_float(box, "ipad");
    float       spacing = pgb_prop_get_float(box, "item-spacing");
    PgPt        avail = flip(box->size, vert);
    avail = pgpt(avail.x - ipad * 2.0f, avail.y - ipad * 2.0f);

    /*
        Compute children size.
        Leave children in (major, minor) order.
    */
    for (pgb_t *c = box->children; c; c = c->next) {
        if (pgb_prop_get_bool(c, "float"))
            continue;

        if (!strcmp("fill", pgb_prop_get(c, "major")))
            nfills++;

        PgPt    size = flip(pgb_measure(c), vert);
        float   between = c == box->children? 0.0f: spacing;
        float   opad = pgb_prop_get_float(c, "opad");
        float   sx = size.x + opad * 2.0f + between;
        c->size = size;
        avail.x -= sx;
    }

    /*
        Assign positions and finalise size now that we know how
        much space is left over for all of the major fills to share.
        After this loop, the children's size are in the correct order.
    */
    float   share = nfills && avail.x > 0? avail.x / nfills: 0.0f;
    float   x = ipad;
    for (pgb_t *c = box->children; c; c = c->next) {

        if (pgb_prop_get_bool(c, "float")) {
            c->size = pgb_restrict(c, flip(c->size, vert));
            continue;
        }

        const char *minor = pgb_prop_get(c, "minor");
        PgPt    size = flip(pgb_measure(c), vert);
        float   between = c == box->children? 0.0f: spacing;
        float   opad = pgb_prop_get_float(c, "opad");
        float   y = !strcmp("start", minor)? opad:
                    !strcmp("center", minor)? opad + (avail.y - c->size.y) * .5f:
                    !strcmp("end", minor)? avail.y - c->size.y:
                    opad;

        if (!strcmp("fill", minor))
            size.y = avail.y;

        if (!strcmp("fill", pgb_prop_get(c, "major")))
            size.x += share;

        x += opad + between;
        y += ipad;

        c->at = flip(pgpt(x, y), vert);
        c->size = pgb_restrict(c, flip(size, vert));

        x += size.x + opad;
        pgb_pack(c);
    }
}


void
pgb_default_draw(pgb_t *box, Pg *g)
{
    for (pgb_t *c = box->children; c; c = c->next) {
        Pg *sub = pg_canvas_new_subcanvas(g, c->at.x, c->at.y, c->size.x, c->size.y);
        pgb_draw(c, sub);
        pg_canvas_commit(sub);
        pg_canvas_free(sub);
    }
}


/*
    Return the ideal content size of this box and its children.
*/
PgPt
pgb_measure(const pgb_t *box)
{
    return box && box->type && box->type->measure
        ? pgb_restrict(box, box->type->measure(box))
        : pgb_default_measure(box);
}


/*
    Pack children into space available in box.
*/
void
pgb_pack(pgb_t *box)
{
    if (box && box->type && box->type->pack)
        box->type->pack(box);
    else
        pgb_default_pack(box);
}


/*
    Restrict box size to integers between min and max.
*/
PgPt
pgb_restrict(const pgb_t *box, PgPt size)
{
    float minx = pgb_prop_get_float(box, "min-x");
    float miny = pgb_prop_get_float(box, "min-y");
    float maxx = pgb_prop_get_float(box, "max-x");
    float maxy = pgb_prop_get_float(box, "max-y");
    if (!box) return pgpt(0, 0);
    return pgpt(ceilf(fminf(maxx, fmaxf(minx, size.x))),
                ceilf(fminf(maxy, fmaxf(miny, size.y))));
}


void
pgb_draw(pgb_t *box, Pg *canvas)
{
    if (box && box->type && box->type->draw)
        box->type->draw(box, canvas);
    else
        pgb_default_draw(box, canvas);
}


pgb_t*
pgb_add(pgb_t *parent, pgb_t *child)
{
    if (!parent || !child)
        return parent;

    pgb_remove(child);
    pgb_t **p = &parent->children;
    while (*p) p = &(*p)->next;
    *p = child;
    child->parent = parent;
    return parent;
}


pgb_t*
pgb_adds(pgb_t *parent, ...)
{
    va_list ap;
    va_start(ap, parent);
    pgb_t *child;
    while ((child = va_arg(ap, pgb_t*)))
        pgb_add(parent, child);
    return parent;
}


void
pgb_remove(pgb_t *child)
{
    if (!child || !child->parent) return;

    pgb_t **p = &child->parent->children;
    while (*p && *p != child) p = &(*p)->next;
    if (*p == child) *p = (*p)->next;
    child->parent = NULL;
}


pgb_t*
pgb_handle(pgb_t *box, const char *event, bool (*handle)(pgb_t *box, void *etc))
{
    if (!box || !event || !handle) return box;

    pgb_handler_t **p = &box->handlers;
    while (*p) p = &(*p)->next;
    *p = pgnew(pgb_handler_t, .event=strdup(event), .handle=handle, .next=0);
    return box;
}


pgb_t*
pgb_handles(pgb_t *box, ...)
{
    va_list ap;
    va_start(ap, box);
    const char *event;

    while ((event = va_arg(ap, const char*))) {
        bool (*handle)(pgb_t *box, void *etc) = va_arg(ap, bool (*)(pgb_t *box, void *etc));
        pgb_handle(box, event, handle);
    }
    va_end(ap);
    return box;
}


bool
pgb_event(pgb_t *box, const char *event, void *etc)
{
    if (!box || !event)
        return false;

    for (pgb_handler_t *i = box->handlers; i; i = i->next)
        if (!strcmp(event, i->event))
            if (i->handle(box, etc))
                return true;
    return false;
}


const char*
pgb_get_type(const pgb_t *box)
{
    return box && box->type && box->type->name? box->type->name: "";
}


static void
init_fonts(void)
{
    if (font) return;

    float dpi = pg_window_get_dpi(NULL).y;

    const char *family = "system-ui, ui-sans-serif, sans-serif, arial, any";
    font = pg_font_find(family, 400, false);
    subtitle_font = pg_font_find(family, 400, false);
    title_font = pg_font_find(family, 600, false);
    monospace_font = pg_font_find("ui-monospace, monospace, courier, any", 400, false);

    float size = 9.0f * dpi / 72.0f;
    pg_font_scale(font, size, 0.0f);
    pg_font_scale(subtitle_font, size * 1.3f, 0.0f);
    pg_font_scale(title_font, size * 1.6f, 0.0f);
    pg_font_scale(monospace_font, size * 0.9f, 0.0f);
}


PgFont*
pgb_font(void)
{
    init_fonts();
    return font;
}


PgFont*
pgb_monospace_font(void)
{
    init_fonts();
    return monospace_font;
}


PgFont*
pgb_subtitle_font(void)
{
    init_fonts();
    return subtitle_font;
}


PgFont*
pgb_title_font(void)
{
    init_fonts();
    return title_font;
}


pgb_t*
pgb_props_set(pgb_t *box, ...)
{
    va_list ap;
    va_start(ap, box);
    pgb_props_set_var(box, va_arg(ap, char*), ap);
    va_end(ap);
    return box;
}


pgb_t*
pgb_props_set_var(pgb_t *box, const char *name, va_list ap)
{
    typedef bool (*handler)(pgb_t *box, void *etc);

    if (!box || !name)
        return box;

    // Add properties and handlers.
    do {
        if (!memcmp("on-", name, 3))
            pgb_handle(box, name + 3, va_arg(ap, handler));
        else if (!strcmp("children", name))
            break;
        else
            pgb_prop_set(box, name, va_arg(ap, char*));
    } while ((name = va_arg(ap, char*)));

    // Add children.
    if (name && !strcmp("children", name)) {
        pgb_t *child;
        while ((child = va_arg(ap, pgb_t*)))
            pgb_add(box, child);
    }
    return box;
}


pgb_t*
pgb_prop_set(pgb_t *box, const char *name, const char *value)
{
    set_prop(&box->props, name, value);
    pgb_update(box);
    return box;
}


pgb_t*
pgb_prop_set_bool(pgb_t *box, const char *name, bool value)
{
    return pgb_prop_set(box, name, value? "true": "false");
}


pgb_t*
pgb_prop_set_float(pgb_t *box, const char *name, float value)
{
    char buf[32];
    snprintf(buf, sizeof buf, "%g", value);
    return pgb_prop_set(box, name, buf);
}


const char*
pgb_prop_get(const pgb_t *box, const char *name)
{
    return get_prop(box->props, name);
}


bool
pgb_prop_get_bool(const pgb_t *box, const char *name)
{
    const char *tmp = get_prop(box->props, name);
    return tmp? !strcmp("true", tmp): false;
}


float
pgb_prop_get_float(const pgb_t *box, const char *name)
{
    const char *tmp = get_prop(box->props, name);
    return tmp? strtod(tmp, NULL): 0.0f;
}


static void
free_props(pgb_prop_t *props)
{
    for (pgb_prop_t *i = props, *next; i; i = next)
        next = i->next,
        free(i->name),
        free(i->value),
        free(i);
}


static void
set_prop(pgb_prop_t **props, const char *name, const char *value)
{
    if (!props || !name) return;

    pgb_prop_t **p = props;
    while (*p && strcmp(name, (*p)->name))
        p = &(*p)->next;

    if (*p) {
        free((*p)->value);
        (*p)->value = value? strdup(value): NULL;
    } else
        *p = pgnew(pgb_prop_t, strdup(name), value? strdup(value): NULL, NULL);
}


static const char*
get_prop(const pgb_prop_t *props, const char *name)
{
    for (const pgb_prop_t *i = props; i; i = i->next)
        if (!strcmp(i->name, name)) return i->value;
    return NULL;
}
