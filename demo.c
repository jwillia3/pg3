#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdatomic.h>
#include <threads.h>
#include <time.h>
#include <pg3.h>

#define FPS 120
#define SCROLLBACK  10000

#define MIN(X, Y) (X < Y? X: Y)
#define MAX(X, Y) (X > Y? X: Y)

static volatile char    **_lines;
static volatile int     _nlines;
static volatile int     _cur;
static volatile float   _font_size = 10.0;
static atomic_bool      paused = false;
static atomic_bool      redraw;
static atomic_bool      terminate;
static mtx_t            lines_mtx;


void append(char *text)
{
    mtx_lock(&lines_mtx);

    if (_nlines >= SCROLLBACK) {
        free((void*) _lines[0]);
        for (int i = 1; i < SCROLLBACK; i++)
            _lines[i - 1] = _lines[i];
        _nlines = SCROLLBACK - 1;
        // _lines = realloc(_lines, (_nlines + SCROLLBACK) * sizeof *_lines);
    }

    _lines[_nlines++] = strdup(text);

    mtx_unlock(&lines_mtx);
}

void draw(Pg *g, PgFont *font)
{
    mtx_lock(&lines_mtx);

    /*
        Copy the state data and release the lock.
        Only work on the copy from this point on.
     */

    pg_scale_font(font, 0.0f, _font_size * pg_dpi().y / 72.0f);

    PgPt    size = pg_size(g);
    float   line_height = pg_font_height(font);
    int     nlines = _nlines;
    int     cur = _cur;
    int     fits = size.y / line_height;
    int     first = MAX(MIN(nlines - fits, cur), 0);
    int     last = MIN(first + fits, nlines);

    char    **copy = malloc((last - first) * sizeof *copy);
    for (int i = 0; i < last - first; i++)
        copy[i] = strdup((void*) _lines[i + first]);

    if (!paused)
        _cur = MAX(0, nlines - fits);

    mtx_unlock(&lines_mtx);


    // Ensure the size is correct since the message loop thread
    // cannot call OpenGL.
    pg_resize(g, size.x, size.y);

    pg_identity(g);

    PgPaint bg = pg_linear(PG_LCHAB, 0.0, 0.0, size.x, 0);
    pg_add_stop(&bg, 0.0, 0.6, 0.1, 0.775, 1.0);
    pg_add_stop(&bg, 0.5, 0.8, 0.1, 0.775, 1.0);
    pg_add_stop(&bg, 1.0, 0.5, 0.1, 0.775, 1.0);

    pg_set_clear(g, &bg);
    pg_clear(g);

    PgPaint fg = pg_solid(PG_LCHAB, 0.025, 0.0, 0.0, 1.0);
    pg_set_fill(g, &fg);

    float   y = 0;
    for (int i = 0; i < last - first; i++) {
        pg_string_path(g, font, 4.0f, y, copy[i]);
        pg_fill(g);
        y += line_height;
        free(copy[i]);
    }
    free(copy);

    bg = pg_solid(PG_LCHAB, 0.75, 0.5, 0.3, 0.25);
    fg = pg_solid(PG_LCHAB, 0.75, 0.5, 0.3, 1.0);
    pg_set_fill(g, &bg);
    pg_set_stroke(g, &fg);
    y = (cur - first) * line_height;
    pg_rounded(g, 0, y, size.x, line_height, 8.0, 8.0);
    pg_fill_stroke(g);



    pg_update();
}

int render_thread(void *_ignore)
{
    (void) _ignore;

    struct timespec sleep_time = {.tv_nsec = 1000000000 / FPS};

    Pg      *g = pg_window(1024, 768, "Demo");

    char    *family_name = "Courier Prime";
    PgFont  *font = pg_find_font(family_name, 400, false);
    if (!font) {
        printf("cannot open font: %s", family_name);
        return 1;
    }

    while (!terminate) {
        while (!redraw && !terminate)
            thrd_sleep(&sleep_time, 0);
        redraw = false;
        draw(g, font);
    }

    return 0;
}

int feed_thread(void *_ignore)
{
    (void) _ignore;

    char    *filename = "war-and-peace.txt";
    FILE    *src = fopen(filename, "rb");
    char    *buf = 0;
    size_t  allocated = 0;

    if (!src) {
        char    tmp[256];
        snprintf(tmp, sizeof tmp, "could not open: %s", filename);
        append(tmp);
        return 1;
    }

    while (!feof(src) && !terminate) {
        size_t  len = getline(&buf, &allocated, src);
        while (len && buf[len - 1] == '\n') {
            buf[len - 1] = 0;
            len--;
        }
        if (!feof(src))
            append(buf);

        if (!paused)
            pg_enqueue((PgEvent) { PG_USER_EVENT });
    }

    fclose(src);
    free(buf);
    return 0;
}

int main(int argc, char **argv)
{
    setvbuf(stdout, 0, _IONBF, 0);

    _lines = malloc(SCROLLBACK * sizeof *_lines);

    thrd_t  feed_tid;
    thrd_t  render_tid;
    thrd_create(&feed_tid, feed_thread, 0);
    thrd_create(&render_tid, render_thread, 0);

    PgEvent evt;
    while (pg_wait(&evt) && !terminate)
        switch (evt.type) {

        case PG_USER_EVENT:
            redraw = true;
            break;

        case PG_REDRAW_EVENT:
            redraw = true;
            break;

        case PG_RESIZE_EVENT:
            redraw = true;
            break;

        case PG_MOUSE_DOWN_EVENT:
            break;

        case PG_KEY_DOWN_EVENT:
            switch (evt.key.key) {

            case ' ':
                paused = !paused;
                redraw = true;
                break;

            case 'W':
                if (evt.key.mods == PG_MOD_CTRL)
                    terminate = true;
                break;

            case PG_KEY_UP:
                _cur--;
                redraw = true;
                break;

            case PG_KEY_DOWN:
                _cur++;
                redraw = true;
                break;

            case PG_KEY_PAGE_UP:
                _cur -= 10;
                redraw = true;
                break;

            case PG_KEY_PAGE_DOWN:
                _cur += 10;
                redraw = true;
                break;

            case PG_KEY_HOME:
                _cur = 0;
                redraw = true;
                break;

            case PG_KEY_END:
                _cur = _nlines;
                redraw = true;
                break;

            }
            break;

        case PG_MOUSE_SCROLL_EVENT:
            if (evt.mouse.mods == PG_MOD_CTRL)
                _font_size *= 1.00 + (evt.mouse.scroll.y * 0.25);
            else {
                _cur -= evt.mouse.scroll.y * 5;
                paused = true;
            }
            redraw = true;
            break;

        case PG_CLOSE_EVENT:
            terminate = true;
            break;
        }

    int _ignore;
    thrd_join(render_tid, &_ignore); // Wait for render before exiting.
}
