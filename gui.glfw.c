#ifdef USE_GLFW

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pg3.h>
#include <internal.h>

#define MAX_QUEUE   64

static volatile PgEvent     queue[MAX_QUEUE];
static volatile int         queue_count;
static volatile int         queue_index;
static volatile bool        is_waiting;
static volatile bool        pending_redraws;
static pthread_mutex_t      queue_mtx;
static bool                 initialized;
static char                 **dropped_files;

static int translate_key(int key);
static int translate_mods(int mods);
static int get_mods(GLFWwindow *win);

bool
pg_enqueue(PgEvent evt)
{
    bool    ok = true;

    pthread_mutex_lock(&queue_mtx);

    if (evt.type == PG_REDRAW_EVENT && pending_redraws)
        /*
            pgb_dirty() calls can cause multiple events to
            be put on the queue. We only need one.
         */
        goto skip;

    if (queue_count < MAX_QUEUE) {
        queue[queue_index] = evt;
        queue_index = (queue_index + 1) % MAX_QUEUE;
        queue_count++;

        if (evt.type == PG_REDRAW_EVENT)
            pending_redraws = true;

        if (is_waiting)
            glfwPostEmptyEvent();
    } else
        ok = false;


    skip:


    pthread_mutex_unlock(&queue_mtx);
    return ok;
}

static
bool
dequeue(PgEvent *evt)
{
    bool ok = true;

    if (!evt)
        return false;

    pthread_mutex_lock(&queue_mtx);

    if (queue_count) {
        int i = queue_index - queue_count;
        if (i < 0)
            i += MAX_QUEUE;

        *evt = queue[i];
        queue_count--;

        if (evt->type == PG_REDRAW_EVENT)
            pending_redraws = false;
    } else
        ok = false;

    pthread_mutex_unlock(&queue_mtx);
    return ok;
}

static
void
resized(GLFWwindow *win, int width, int height)
{
    Pg *g = glfwGetWindowUserPointer(win);
    glfwMakeContextCurrent(win);
    pg_resize(g, width, height);
    pg_enqueue_resized(g, width, height);
}

static
void
update(GLFWwindow *win)
{
    glfwMakeContextCurrent(win);
    pg_enqueue_redraw(glfwGetWindowUserPointer(win));
}

static
void
closed(GLFWwindow *win)
{
    pg_enqueue_closed(glfwGetWindowUserPointer(win));
}

static
void
key_pressed(GLFWwindow *win, int key, int scancode, int action, int mods)
{
    (void) scancode;
    Pg *g = glfwGetWindowUserPointer(win);
    key = translate_key(key);
    mods = translate_mods(mods);
    if (action == GLFW_RELEASE)
        pg_enqueue_key_up(g, key, mods);
    else
        pg_enqueue_key_down(g, key, mods);
}

static
void
char_pressed(GLFWwindow *win, unsigned codepoint)
{
    pg_enqueue_char(glfwGetWindowUserPointer(win), codepoint);
}

static
void
button_pressed(GLFWwindow *win, int button, int action, int mods)
{
    Pg *g = glfwGetWindowUserPointer(win);
    double x, y;
    glfwGetCursorPos(win, &x, &y);
    mods = translate_mods(mods);
    if (action == GLFW_RELEASE)
        pg_enqueue_mouse_up(g, x, y, button, mods);
    else
        pg_enqueue_mouse_down(g, x, y, button, mods);
}

static
void
files_dropped(GLFWwindow *win, int npaths, const char **paths)
{
    Pg *g = glfwGetWindowUserPointer(win);

    if (dropped_files) {
        for (char **i = dropped_files; *i; i++)
            free(*i);
        free(dropped_files);
    }

    dropped_files = malloc((npaths + 1) * sizeof *paths);
    for (int i = 0; i < npaths; i++)
        dropped_files[i] = strdup(paths[i]);
    dropped_files[npaths] = 0;

    pg_enqueue_files_dropped(g, npaths, (const char**) dropped_files);
}

static
void
mouse_moved(GLFWwindow *win, double x, double y)
{
    pg_enqueue_mouse_move(glfwGetWindowUserPointer(win), x, y, get_mods(win));
}

static
void
scroll(GLFWwindow *win, double dx, double dy)
{
    Pg *g = glfwGetWindowUserPointer(win);
    double x, y;
    glfwGetCursorPos(win, &x, &y);
    int mods = get_mods(win);
    pg_enqueue_scrolled(g, x, y, dx, dy, mods);
}

bool
pg_init_gui(void)
{
    if (initialized)
        return true;

    if (!glfwInit())
        return false;
    glfwWindowHint(GLFW_SAMPLES, 8);
    glfwWindowHint(GLFW_DOUBLEBUFFER, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    pthread_mutex_init(&queue_mtx, 0);

    return true;
}

PgPt
pg_dpi(void)
{
    if (!glfwInit())
        return PgPt(96.0f, 96.0f);

    float x, y;
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &x, &y);
    return PgPt(96.0f * x, 96.0f * y);
}


void
pg_set_window_title(Pg *g, const char *title)
{
    if (!g || !pg_get_sys(g) || !title) return;
    glfwSetWindowTitle(pg_get_sys(g), title);
}


Pg*
pg_window(float width, float height, const char *title)
{
    if (width < 0.0f || height < 0.0f)
        return 0;

    if (!pg_init_gui())
        return 0;

    if (width <= 1.0f || height <= 1.0f) {
        if (width == 0.0f)
            width = 0.5f;

        if (height == 0.0f)
            height = 0.5f;

        GLFWmonitor *monitor = glfwGetPrimaryMonitor();
        int         msx, msy;

        glfwGetMonitorWorkarea(monitor, 0, 0, &msx, &msy);

        if (width <= 1.0f)
            width = msx * width;

        if (height <= 1.0f)
            height = msy * height;
    }

    GLFWwindow *win = glfwCreateWindow(width, height, title, 0, 0);
    glfwMakeContextCurrent(win);
    glfwSwapInterval(0);
    glfwSetFramebufferSizeCallback(win, resized);
    glfwSetWindowRefreshCallback(win, update);
    glfwSetWindowCloseCallback(win, closed);
    glfwSetKeyCallback(win, key_pressed);
    glfwSetMouseButtonCallback(win, button_pressed);
    glfwSetCursorPosCallback(win, mouse_moved);
    glfwSetScrollCallback(win, scroll);
    glfwSetCharCallback(win, char_pressed);
    glfwSetDropCallback(win, files_dropped);

    glewInit();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    Pg *g = pg_opengl((float) width, (float) height);
    pg_set_sys(g, win);
    glfwSetWindowUserPointer(win, g);
    return g;
}

bool
pg_wait(PgEvent *evt)
{
    PgEvent ignore;
    if (!evt)
        evt = &ignore;

    *evt = (PgEvent) { PG_NO_EVENT, 0, {0} };

    if (dequeue(evt))
        return true;

    is_waiting = true;
    glfwWaitEvents();
    is_waiting = false;

    if (dequeue(evt))
        return true;
    return true;
}

void
pg_update(Pg *g)
{
    if (!g)
        return;

    glfwSwapBuffers(g->sys);
}

static int key_map[GLFW_KEY_LAST + 1] = {
    [GLFW_KEY_WORLD_1] = 0,
    [GLFW_KEY_WORLD_2] = 0,
    [GLFW_KEY_ESCAPE] = PG_KEY_ESCAPE,
    [GLFW_KEY_ENTER] = PG_KEY_ENTER,
    [GLFW_KEY_TAB] = PG_KEY_TAB,
    [GLFW_KEY_BACKSPACE] = PG_KEY_BACKSPACE,
    [GLFW_KEY_INSERT] = PG_KEY_INSERT,
    [GLFW_KEY_DELETE] = PG_KEY_DELETE,
    [GLFW_KEY_RIGHT] = PG_KEY_RIGHT,
    [GLFW_KEY_LEFT] = PG_KEY_LEFT,
    [GLFW_KEY_DOWN] = PG_KEY_DOWN,
    [GLFW_KEY_UP] = PG_KEY_UP,
    [GLFW_KEY_PAGE_UP] = PG_KEY_PAGE_UP,
    [GLFW_KEY_PAGE_DOWN] = PG_KEY_PAGE_DOWN,
    [GLFW_KEY_HOME] = PG_KEY_HOME,
    [GLFW_KEY_END] = PG_KEY_END,
    [GLFW_KEY_CAPS_LOCK] = PG_KEY_CAPS_LOCK,
    [GLFW_KEY_SCROLL_LOCK] = PG_KEY_SCROLL_LOCK,
    [GLFW_KEY_NUM_LOCK] = PG_KEY_NUM_LOCK,
    [GLFW_KEY_PRINT_SCREEN] = PG_KEY_PRINT_SCREEN,
    [GLFW_KEY_PAUSE] = PG_KEY_PAUSE,
    [GLFW_KEY_F1] = PG_KEY_F1,
    [GLFW_KEY_F2] = PG_KEY_F2,
    [GLFW_KEY_F3] = PG_KEY_F3,
    [GLFW_KEY_F4] = PG_KEY_F4,
    [GLFW_KEY_F5] = PG_KEY_F5,
    [GLFW_KEY_F6] = PG_KEY_F6,
    [GLFW_KEY_F7] = PG_KEY_F7,
    [GLFW_KEY_F8] = PG_KEY_F8,
    [GLFW_KEY_F9] = PG_KEY_F9,
    [GLFW_KEY_F10] = PG_KEY_F10,
    [GLFW_KEY_F11] = PG_KEY_F11,
    [GLFW_KEY_F12] = PG_KEY_F12,
    [GLFW_KEY_F13] = 0,
    [GLFW_KEY_F14] = 0,
    [GLFW_KEY_F15] = 0,
    [GLFW_KEY_F16] = 0,
    [GLFW_KEY_F17] = 0,
    [GLFW_KEY_F18] = 0,
    [GLFW_KEY_F19] = 0,
    [GLFW_KEY_F20] = 0,
    [GLFW_KEY_F21] = 0,
    [GLFW_KEY_F22] = 0,
    [GLFW_KEY_F23] = 0,
    [GLFW_KEY_F24] = 0,
    [GLFW_KEY_F25] = 0,
    [GLFW_KEY_KP_0] = PG_KEY_KP_0,
    [GLFW_KEY_KP_1] = PG_KEY_KP_1,
    [GLFW_KEY_KP_2] = PG_KEY_KP_2,
    [GLFW_KEY_KP_3] = PG_KEY_KP_3,
    [GLFW_KEY_KP_4] = PG_KEY_KP_4,
    [GLFW_KEY_KP_5] = PG_KEY_KP_5,
    [GLFW_KEY_KP_6] = PG_KEY_KP_6,
    [GLFW_KEY_KP_7] = PG_KEY_KP_7,
    [GLFW_KEY_KP_8] = PG_KEY_KP_8,
    [GLFW_KEY_KP_9] = PG_KEY_KP_9,
    [GLFW_KEY_KP_DECIMAL] = PG_KEY_KP_DECIMAL,
    [GLFW_KEY_KP_DIVIDE] = PG_KEY_KP_DIVIDE,
    [GLFW_KEY_KP_MULTIPLY] = PG_KEY_KP_MULTIPLY,
    [GLFW_KEY_KP_SUBTRACT] = PG_KEY_KP_SUBTRACT,
    [GLFW_KEY_KP_ADD] = PG_KEY_KP_ADD,
    [GLFW_KEY_KP_ENTER] = PG_KEY_KP_ENTER,
    [GLFW_KEY_KP_EQUAL] = PG_KEY_KP_EQUAL,
    [GLFW_KEY_LEFT_SHIFT] = PG_KEY_SHIFT_LEFT,
    [GLFW_KEY_LEFT_CONTROL] = PG_KEY_CTRL_LEFT,
    [GLFW_KEY_LEFT_ALT] = PG_KEY_ALT_LEFT,
    [GLFW_KEY_LEFT_SUPER] = PG_KEY_WIN_LEFT,
    [GLFW_KEY_RIGHT_SHIFT] = PG_KEY_SHIFT_RIGHT,
    [GLFW_KEY_RIGHT_CONTROL] = PG_KEY_CTRL_RIGHT,
    [GLFW_KEY_RIGHT_ALT] = PG_KEY_ALT_RIGHT,
    [GLFW_KEY_RIGHT_SUPER] = PG_KEY_WIN_RIGHT,
    [GLFW_KEY_MENU] = PG_KEY_MENU,
    [' '] = ' ',
    ['\''] = '\'',
    [','] = ',',
    ['-'] = '-',
    ['.'] = '.',
    ['/'] = '/',
    [';'] = ';',
    ['='] = '=',
    ['['] = '[',
    ['\\'] = '\\',
    [']'] = '[',
    ['`'] = '`',
    ['0'] = '0',
    ['1'] = '1',
    ['2'] = '2',
    ['3'] = '3',
    ['4'] = '4',
    ['5'] = '5',
    ['6'] = '6',
    ['7'] = '7',
    ['8'] = '8',
    ['9'] = '9',
    ['A'] = 'A',
    ['B'] = 'B',
    ['C'] = 'C',
    ['D'] = 'D',
    ['E'] = 'E',
    ['F'] = 'F',
    ['G'] = 'G',
    ['H'] = 'H',
    ['I'] = 'I',
    ['J'] = 'J',
    ['K'] = 'K',
    ['L'] = 'L',
    ['M'] = 'M',
    ['N'] = 'N',
    ['O'] = 'O',
    ['P'] = 'P',
    ['Q'] = 'Q',
    ['R'] = 'R',
    ['S'] = 'S',
    ['T'] = 'T',
    ['U'] = 'U',
    ['V'] = 'V',
    ['W'] = 'W',
    ['X'] = 'X',
    ['Y'] = 'Y',
    ['Z'] = 'Z',
};


static
int
translate_key(int key)
{
    if (key == -1)
        return 0;
    return key_map[key];
}


static
int
translate_mods(int mods)
{
    return mods;
}


static
int
get_mods(GLFWwindow *win)
{
    bool shift = glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) || glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT);
    bool ctrl = glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) || glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL);
    bool alt = glfwGetKey(win, GLFW_KEY_LEFT_ALT) || glfwGetKey(win, GLFW_KEY_RIGHT_ALT);
    bool sup = glfwGetKey(win, GLFW_KEY_LEFT_SUPER) || glfwGetKey(win, GLFW_KEY_RIGHT_SUPER);
    bool caps = glfwGetKey(win, GLFW_KEY_CAPS_LOCK);
    bool num = glfwGetKey(win, GLFW_KEY_NUM_LOCK);
    return (shift? PG_MOD_SHIFT: 0) +
           (ctrl? PG_MOD_CTRL: 0) +
           (alt? PG_MOD_WIN: 0) +
           (sup? PG_MOD_WIN: 0) +
           (caps? PG_MOD_CAPS: 0) +
           (num? PG_MOD_NUM: 0);
}

char*
pg_get_clipboard(void)
{
    const char *original = glfwGetClipboardString(0);
    return original? strdup(original): 0;
}

void
pg_set_clipboard(const char *text)
{
    glfwSetClipboardString(0, text);
}

#endif
