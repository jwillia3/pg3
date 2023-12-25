#ifdef USE_XLIB

#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <EGL/egl.h>
#include <GL/glew.h>
#include <GL/glx.h>
#include <pg3/pg.h>
#include <pg3/pg-internal-window.h>

static Display          *xdisplay;
static Window           xwindow;
static Atom             WM_PROTOCOLS;
static Atom             WM_DELETE_WINDOW;
static Atom             IGNORE_MESSAGE;
static PgWindow         *window;
static EGLDisplay       egl_display;
static EGLContext       egl_context;
static EGLSurface       egl_surface;
#define mouse_motion_cooldown (1/60.0)
static const int        event_mask =  ExposureMask |
                                      PointerMotionMask |
                                      KeyPressMask |
                                      KeyReleaseMask |
                                      ButtonPressMask |
                                      ButtonReleaseMask |
                                      PointerMotionMask |
                                      StructureNotifyMask |
                                      SubstructureNotifyMask;


static
Window
setup_protocols(Window win, const char *title)
{
    if (!win)
        return 0;

    IGNORE_MESSAGE = XInternAtom(xdisplay, "__IGNORE_MESSAGE", false);

    WM_PROTOCOLS = XInternAtom(xdisplay, "WM_PROTOCOLS", false);
    WM_DELETE_WINDOW = XInternAtom(xdisplay, "WM_DELETE_WINDOW", false);
    XSetWMProtocols(xdisplay, win, &WM_DELETE_WINDOW, 1);
    XStoreName(xdisplay, win, title);
    XMapWindow(xdisplay, win);
    return win;
}


static
Window
native_with_visual(unsigned width, unsigned height, const char *title, XVisualInfo *vi)
{
    XSetWindowAttributes attr = {
                .event_mask = ExposureMask |
                              PointerMotionMask |
                              KeyPressMask |
                              KeyReleaseMask |
                              ButtonPressMask |
                              ButtonReleaseMask |
                              PointerMotionMask |
                              StructureNotifyMask |
                              SubstructureNotifyMask,
                .border_pixel = 0,
                .colormap = XCreateColormap(xdisplay,
                                            RootWindow(xdisplay, vi->screen),
                                            vi->visual,
                                            AllocNone),
                .background_pixmap = None,
    };

    Window win = XCreateWindow(xdisplay, RootWindow(xdisplay, vi->screen),
                               0, 0, width, height,
                               0, vi->depth, InputOutput,
                               vi->visual,
                               CWBorderPixel | CWColormap | CWEventMask,
                               &attr);
    return setup_protocols(win, title);
}


static
bool
setup_glx(unsigned width, unsigned height, const char *title)
{
    int ignore;
    if (!glXQueryVersion(xdisplay, &ignore, &ignore))
        return false;

    static int  vattr[] = {
                    GLX_X_RENDERABLE,   true,
                    GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                    GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
                    GLX_STENCIL_SIZE,   8,
                    GLX_DOUBLEBUFFER,   true,
                    GLX_SAMPLE_BUFFERS, 1,
                    GLX_SAMPLES,        8,
                    None
                };

    int             nconfs;
    GLXFBConfig    *confs;
    GLXFBConfig    *best;

    confs = glXChooseFBConfig(xdisplay, DefaultScreen(xdisplay), vattr, &nconfs);
    if (!confs)
        return false;


    /* Get the configuration with the most samples. */
    best = confs;
    for (GLXFBConfig *i = confs; i < confs + nconfs; i++) {
        int max, cur;
        glXGetFBConfigAttrib(xdisplay, *best, GLX_SAMPLES, &max);
        glXGetFBConfigAttrib(xdisplay, *i, GLX_SAMPLES, &cur);
        if (cur > max)
            best = i;
    }

    XVisualInfo *vi = glXGetVisualFromFBConfig(xdisplay, *best);
    xwindow = native_with_visual(width, height, title, vi);

    if (xwindow) {
        GLXContext ctx;
        ctx = glXCreateNewContext(xdisplay, *best, GLX_RGBA_TYPE, 0, true);
        XSync(xdisplay, false);
        glXMakeCurrent(xdisplay, xwindow, ctx);
    }

    XFree(vi);
    XFree(confs);
    return true;
}


static
Window
native_window(unsigned width, unsigned height, const char *title)
{
    XSetWindowAttributes attr = { .event_mask = event_mask };
    Window win = XCreateWindow(xdisplay, DefaultRootWindow(xdisplay),
                               0, 0, width, height,
                               0, CopyFromParent, InputOutput,
                               CopyFromParent, CWEventMask, &attr);
    return setup_protocols(win, title);
}


static
bool
setup_egl(unsigned width, unsigned height, const char *title)
{
    static const EGLint attrs[] = {
        EGL_SAMPLES,        8,
        EGL_STENCIL_SIZE,   8,
        EGL_NONE,
    };
    EGLConfig conf;
    int nconfs;
    eglBindAPI(EGL_OPENGL_API);
    egl_display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(egl_display, NULL, NULL);
    eglChooseConfig(egl_display, attrs, &conf, 1, &nconfs);
    egl_context = eglCreateContext(egl_display, conf, EGL_NO_CONTEXT, NULL);
    xwindow = native_window(width, height, title);
    egl_surface = eglCreateWindowSurface(egl_display, conf, xwindow, NULL);
    eglSwapInterval(egl_display, 0);
    eglMakeCurrent(egl_display, egl_surface, egl_surface, egl_context);

    if (!egl_surface) {
        eglDestroyContext(egl_display, egl_context);
        XDestroyWindow(xdisplay, xwindow);
        return false;
    }

    return true;
}


PgWindow*
_pg_window_open(unsigned width, unsigned height, const char *title)
{
    if (window)
        /*
            Only one window is supported.
         */
        return NULL;

    if (!xdisplay) {
        xdisplay = XOpenDisplay(NULL);

        if (!xdisplay)
            return NULL;
    }

    if (!setup_egl(width, height, title) &&
        !setup_glx(width, height, title))
        return NULL;

    window = pgnew(PgWindow,
                   .g = pg_canvas_new_opengl(1, 1),
                   .width = width,
                   .height = height);
    return window;
}


void
pg_window_update(PgWindow *win)
{
    if (!win)
        return;
    if (!egl_context) {
        glFlush();
        glXSwapBuffers(xdisplay, xwindow);
    }
    else
        eglSwapBuffers(egl_display, egl_surface);
}


void
pg_window_queue_update(PgWindow *win)
{
    if (!win)
        return;
    XClearArea(xdisplay, xwindow, 0, 0, 0, 0, true);
}


void
pg_window_queue_dummy(PgWindow *win)
{
    if (!win)
        return;
    XClientMessageEvent e = {
        .type = ClientMessage,
        .message_type = IGNORE_MESSAGE,
        .format = 8
    };
    XSendEvent(xdisplay, xwindow, false, 0, (XEvent*) &e);
}


void
_pg_window_close(PgWindow *win)
{
    if (win) {
        XClientMessageEvent e = {
            .type = ClientMessage,
            .message_type = WM_PROTOCOLS,
            .format = 32,
            .data.l[0] = WM_DELETE_WINDOW,
        };
        XSendEvent(xdisplay, xwindow, false, 0, (XEvent*) &e);
    }
}


void
_pg_window_free(PgWindow *win)
{
    if (win == window)
        window = NULL;
}


void
_pg_window_set_size(PgWindow *win, unsigned width, unsigned height)
{
    if (win == window)
        XResizeWindow(xdisplay, xwindow, width, height);
}

void
_pg_window_set_title(PgWindow *win, const char *title)
{
    if (win == window)
        XStoreName(xdisplay, xwindow, title);
}


static const char* name_mods(unsigned state);
static const char* name_key(KeySym keysym);
static const char* name_key_chord(unsigned state, KeySym keysym);
static const char* name_button_chord(unsigned state, unsigned button);


PgWindowEvent*
pg_window_event_wait(void)
{
    redo:

    if (!xdisplay || !xwindow)
        return NULL;

    if (window->queued) {
        window->queued = false;
        window->e = window->q;
        return &window->e;
    }

    PgWindowEvent   *e = &window->e;
    XEvent          xev;
    KeySym          keysym;
    PgPt            sz;
    struct timespec ts;
    double          now;

    const char      *name;
    unsigned        state;


    XNextEvent(xdisplay, &xev);

    memset(e, 0, sizeof *e);

    switch (xev.type) {

    case Expose:
        if (xev.xexpose.count == 0)
            /*
                Multiple exposes may come. Only post message on last expose.
             */
            e->any = (PgWindowEventAny) { window, PG_EVENT_PAINT };
        return e;

    case ConfigureNotify:

        sz = pg_canvas_get_size(window->g);

        if (xev.xconfigure.width != sz.x || xev.xconfigure.height != sz.y) {
            window->width = xev.xconfigure.width;
            window->height = xev.xconfigure.height;
            pg_canvas_set_size(window->g,
                               xev.xconfigure.width,
                               xev.xconfigure.height);
            pg_window_queue_update(window);
            e->resized = (PgWindowEventResized) {
                window,
                PG_EVENT_RESIZED,
                .width = xev.xconfigure.width,
                .height = xev.xconfigure.height,
            };
        }
        return e;

    case KeyPress:
    case KeyRelease:

        // Lookup unmodified key (e.g. "1" not "!" when "Shift+1" is pressed).
        state = xev.xkey.state;
        xev.xkey.state = 0;
        keysym = XLookupKeysym(&xev.xkey, 0);
        xev.xkey.state = state;

        name = name_key_chord(xev.xkey.state, keysym);

        if (xev.type == KeyPress) {

            /*
                Look up ASCII.
                Abort if any modifier besides shift is down.
             */

            unsigned    disqualifier = Mod1Mask | ControlMask | Mod3Mask |
                                       Mod4Mask | Mod5Mask;

            if (!(xev.xkey.state & disqualifier)) {
                char            buf[8] = { 0 };
                int             nchars = XLookupString(&xev.xkey, buf, 8, NULL, 0);

                if (nchars < 8 && *buf >= 32 && *buf < 0x7f) {
                    buf[nchars] = 0;
                    window->queued = true,
                    window->q.text = (PgWindowEventText) {
                        window,
                        PG_EVENT_TEXT,
                        .text = window->q.text.tmp,
                    };
                    memcpy(window->q.text.tmp, buf, nchars);
                }
            }

            e->key = (PgWindowEventKey) {
                window,
                PG_EVENT_KEY_DOWN,
                .key = name,
            };
            return e;
        }

        e->key = (PgWindowEventKey) {
            window,
            PG_EVENT_KEY_UP,
            .key = name,
        };
        return e;

    case ClientMessage:
        if (xev.xclient.data.l[0] == (long) WM_DELETE_WINDOW) {
            e->any = (PgWindowEventAny) { window, PG_EVENT_CLOSED };
            xwindow = 0;
            return e;
        }
        return e;

    case ButtonPress:
        if (xev.xbutton.button == 4 || xev.xbutton.button == 5)
            e->mouse = (PgWindowEventMouse) {
                window,
                PG_EVENT_MOUSE_WHEEL,
                .x = xev.xbutton.x,
                .y = xev.xbutton.y,
                .wheel = xev.xbutton.button == 4? -1: 1,
            };
        else
            e->mouse = (PgWindowEventMouse) {
                window,
                PG_EVENT_MOUSE_DOWN,
                .x = xev.xbutton.x,
                .y = xev.xbutton.y,
                .button = name_button_chord(xev.xbutton.state, xev.xbutton.button),
            };
        return e;

    case ButtonRelease:
        e->mouse = (PgWindowEventMouse) {
            window,
            PG_EVENT_MOUSE_UP,
            .x = xev.xbutton.x,
            .y = xev.xbutton.y,
            .button = name_button_chord(xev.xbutton.state, xev.xbutton.button),
        };
        return e;

    case MotionNotify:
        /*
            Throttle mouse motion updates to a given frame rate.
        */
        clock_gettime(CLOCK_MONOTONIC, &ts);
        now = ts.tv_sec + ts.tv_nsec * 1.0e-9;
        if (now < window->last_motion + mouse_motion_cooldown)
            goto redo;
        window->last_motion = now;

        e->mouse = (PgWindowEventMouse) {
            window,
            PG_EVENT_MOUSE_MOVED,
            .x = xev.xmotion.x,
            .y = xev.xmotion.y,
        };
        return e;


    case DestroyNotify:
        e->any = (PgWindowEventAny) { window, PG_EVENT_CLOSED };
        xwindow = 0;
        return e;

    }

    goto redo;
}


PgPt
_pg_window_get_dpi_system(PgWindow *win)
{
    (void) win;

    // Let the platform handle this.
    return pgpt(0, 0);
}


static
const char*
name_mods(unsigned state)
{
    static const char *desc[] = {
        "",
        "Ctrl",
        "Shift",
        "Ctrl+Shift",

        "Alt",
        "Ctrl+Alt",
        "Alt+Shift",
        "Ctrl+Alt+Shift",

        "Win",
        "Ctrl+Win",
        "Win+Shift",
        "Ctrl+Win+Shift",

        "Alt+Win",
        "Ctrl+Alt+Win",
        "Alt+Win+Shift",
        "Ctrl+Alt+Win+Shift",
    };

    bool ctrl = state & ControlMask;
    bool shift = state & ShiftMask;
    bool alt = state & Mod1Mask;
    bool win = state & Mod4Mask;
    return desc[ctrl + shift*2 + alt*4 + win*8];
}


static
const char*
name_key(KeySym keysym)
{
    switch (keysym) {
    case ' ':               return "Space";
    case XK_BackSpace:      return "Backspace";
    case XK_Tab:            return "Tab";
    case XK_Linefeed:       return "Linefeed";
    case XK_Clear:          return "Clear";
    case XK_Return:         return "Enter";
    case XK_Pause:          return "Pause";
    case XK_Scroll_Lock:    return "ScrollLock";
    case XK_Sys_Req:        return "SysReq";
    case XK_Escape:         return "Escape";
    case XK_Delete:         return "Delete";

    case XK_Home:           return "Home";
    case XK_Left:           return "Left";
    case XK_Up:             return "Up";
    case XK_Right:          return "Right";
    case XK_Down:           return "Down";
    case XK_Page_Up:        return "PageUp";
    case XK_Page_Down:      return "PageDown";
    case XK_End:            return "End";
    case XK_Begin:          return "Begin";

    case XK_Select:         return "Select";
    case XK_Print:          return "Print";
    case XK_Execute:        return "Run";
    case XK_Insert:         return "Insert";
    case XK_Undo:           return "Undo";
    case XK_Redo:           return "Redo";
    case XK_Menu:           return "Menu";
    case XK_Find:           return "Find";
    case XK_Cancel:         return "Cancel";
    case XK_Help:           return "Help";
    case XK_Break:          return "Break";
    case XK_Mode_switch:    return "ModeSwitch";
    case XK_Num_Lock:       return "NumLock";

    case XK_KP_Space:       return "KeypadSpace";
    case XK_KP_Tab:         return "KeypadTab";
    case XK_KP_Enter:       return "KeypadEnter";
    case XK_KP_F1:          return "KeypadF1";
    case XK_KP_F2:          return "KeypadF2";
    case XK_KP_F3:          return "KeypadF3";
    case XK_KP_F4:          return "KeypadF4";
    case XK_KP_Home:        return "KeypadHome";
    case XK_KP_Left:        return "KeypadLeft";
    case XK_KP_Up:          return "KeypadUp";
    case XK_KP_Right:       return "KeypadRight";
    case XK_KP_Down:        return "KeypadDown";
    case XK_KP_Page_Up:     return "KeypadPageUp";
    case XK_KP_Page_Down:   return "KeypadPageDown";
    case XK_KP_End:         return "KeypadEnd";
    case XK_KP_Begin:       return "KeypadBegin";
    case XK_KP_Insert:      return "KeypadInsert";
    case XK_KP_Delete:      return "KeypadDelete";
    case XK_KP_Equal:       return "KeypadEqual";
    case XK_KP_Multiply:    return "KeypadMultiply";
    case XK_KP_Add:         return "KeypadAdd";
    case XK_KP_Separator:   return "KeypadSeparator";
    case XK_KP_Subtract:    return "KeypadSubtract";
    case XK_KP_Decimal:     return "KeypadDecimal";
    case XK_KP_Divide:      return "KeypadDivide";

    case XK_KP_0:           return "Keypad0";
    case XK_KP_1:           return "Keypad1";
    case XK_KP_2:           return "Keypad2";
    case XK_KP_3:           return "Keypad3";
    case XK_KP_4:           return "Keypad4";
    case XK_KP_5:           return "Keypad5";
    case XK_KP_6:           return "Keypad6";
    case XK_KP_7:           return "Keypad7";
    case XK_KP_8:           return "Keypad8";
    case XK_KP_9:           return "Keypad9";

    case XK_F1:             return "F1";
    case XK_F2:             return "F2";
    case XK_F3:             return "F3";
    case XK_F4:             return "F4";
    case XK_F5:             return "F5";
    case XK_F6:             return "F6";
    case XK_F7:             return "F7";
    case XK_F8:             return "F8";
    case XK_F9:             return "F9";
    case XK_F10:            return "F10";
    case XK_F11:            return "F11";
    case XK_F12:            return "F12";
    case XK_F13:            return "F13";
    case XK_F14:            return "F14";
    case XK_F15:            return "F15";
    case XK_F16:            return "F16";
    case XK_F17:            return "F17";
    case XK_F18:            return "F18";
    case XK_F19:            return "F19";
    case XK_F20:            return "F20";
    case XK_F21:            return "F21";
    case XK_F22:            return "F22";
    case XK_F23:            return "F23";
    case XK_F24:            return "F24";

    case XK_Shift_L:        return "LeftShift";
    case XK_Shift_R:        return "RightShift";
    case XK_Control_L:      return "LeftControl";
    case XK_Control_R:      return "RightControl";
    case XK_Caps_Lock:      return "CapsLock";
    case XK_Shift_Lock:     return "ShiftLock";
    case XK_Meta_L:         return "LeftMeta";
    case XK_Meta_R:         return "RightMeta";
    case XK_Alt_L:          return "LeftAlt";
    case XK_Alt_R:          return "RightAlt";
    case XK_Super_L:        return "LeftWin";
    case XK_Super_R:        return "RightWin";
    case XK_Hyper_L:        return "LeftHyper";
    case XK_Hyper_R:        return "RightHyper";

    case XK_Multi_key:      return "Compose";
    case XK_SingleCandidate: return "SingleCandidate";
    case XK_MultipleCandidate: return "MultipleCandidate";
    case XK_PreviousCandidate: return "PreviousCandidate";
    case XK_Kanji:          return "Kanji";
    case XK_Muhenkan:       return "CancelConversion";
    case XK_Henkan_Mode:    return "TranslationMode";
    case XK_Romaji:         return "Romaji";
    case XK_Hiragana:       return "Hiragana";
    case XK_Katakana:       return "Katakana";
    case XK_Hiragana_Katakana: return "Kana";
    case XK_Zenkaku:        return "FullWidth";
    case XK_Hankaku:        return "HalfWidth";
    case XK_Zenkaku_Hankaku: return "FullWidthToggle";
    case XK_Touroku:        return "AddToDictionary";
    case XK_Massyo:         return "DeleteFromDictionary";
    case XK_Kana_Lock:      return "KanaLock";
    case XK_Kana_Shift:     return "KanaShift";
    case XK_Eisu_Shift:     return "AlphanumericShift";
    case XK_Eisu_toggle:    return "AlphanumericToggle";
    case XK_Kanji_Bangou:   return "KanjiNumber";

    default:
        if (keysym >= 0x20 && keysym < 0x7f) {
            static char tmp[2];
            *tmp = toupper(keysym);
            return tmp;
        }

        return "Unknown";
    }
}


static
const char*
name_key_chord(unsigned state, KeySym keysym)
{
    const char  *prefix = name_mods(state);
    const char  *suffix = name_key(keysym);

    if (!*prefix)
        return suffix;

    static char buf[128];
    snprintf(buf, sizeof buf, "%s+%s", prefix, suffix);
    return buf;
}


static
const char*
name_button(unsigned button)
{
    if (button == 1)    return "LeftButton";
    if (button == 3)    return "RightButton";
    if (button == 2)    return "MiddleButton";
    static char buf[16];
    snprintf(buf, sizeof buf, "Button%d", button);
    return buf;
}


static
const char*
name_button_chord(unsigned state, unsigned button)
{
    const char  *prefix = name_mods(state);
    const char  *suffix = name_button(button);

    if (!*prefix)
        return suffix;

    static char buf[128];
    snprintf(buf, sizeof buf, "%s+%s", prefix, suffix);
    return buf;
}


#endif
