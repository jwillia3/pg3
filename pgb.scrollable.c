#include <stdlib.h>
#include <string.h>

#include <pg3.h>
#include <pgbox.h>
#include <internal.h>


typedef struct Info Info;
struct Info {
    Pgb     *content;
    Pgb     *hscroll;
    Pgb     *vscroll;
    Pgb     *top;
};


static
void
scrolled(Pgb *box, float value)
{
    (void) box, (void) value;
    Info *info = pgb_get_user(box);

    if (info->hscroll == box)
        info->content->x = -value * info->content->cx + info->vscroll->sx;
    else
        info->content->y = -value * info->content->cy;

    pgb_dirty(info->top);
}


Pgb*
pgb_scrollable(int cx, int cy, Pgb *content, PgbFlags flags)
{
    if (!content)
        return 0;
    Info *info = new(Info,
                     .content = content,
                     .hscroll = 0,
                     .vscroll = 0,
                     .top = 0);
    info->hscroll = pgb_scrollbar(true, 0, scrolled, PGB_FILL_CROSS);
    info->vscroll = pgb_scrollbar(false, 0, scrolled, PGB_FILL_CROSS);

    info->top = pgb_bordered(flags, (Pgb*[]) {
        pgb_hgrouped(PGB_FILL, (Pgb*[]) {
            content,
            info->vscroll,
            0 }),
        info->hscroll,
        0 });

    content->flags |= PGB_FIXED_POS;
    content->x = info->vscroll->cx;
    content->y = 0;

    info->hscroll->cx = cx;
    info->vscroll->cy = cy;

    pgb_set_user(info->hscroll, info);
    pgb_set_user(info->vscroll, info);
    pgb_set_user(info->top, info);
    return info->top;
}
