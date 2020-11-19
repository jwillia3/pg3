#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/Xresource.h>
#include <GL/glew.h>
#include <GL/glx.h>
#include <pg3.h>

#ifdef USE_GLX



static Atom         WM_PROTOCOLS;
static Atom         WM_DELETE_WINDOW;
static Atom         RESOURCE_MANAGER;

static float        dpi_x;
static float        dpi_y;

static Display      *display;
static Screen       *screen;
static Window       window;
static GLXContext   gl;
static Pg           *g;
static bool         redraw;
static float        _mouse_x;
static float        _mouse_y;
static unsigned     _buttons;
static float        _wheel;
static unsigned     _mods;
static int32_t      _key;




static void
qt_overrides(void)
{
    char *val = getenv("QT_AUTO_SCREEN_SCALE_FACTOR");

    if (val) {
        float dpi = 96.0f * (float) strtod(val, 0);

        if (dpi > 0.0f) {
            dpi_x = dpi;
            dpi_y = dpi;
        }
    }
}


static void
gdk_overrides(void)
{
    char *scale = getenv("GDK_SCALE");
    char *dpi = getenv("GDK_DPI_SCALE");

    if (scale || dpi) {
        float x = scale? (float) strtod(scale, 0): 1.0f;
        float y = dpi? (float) strtod(dpi, 0): 1.0f;
        float value = x * y;

        if (value > 0.0f) {
            dpi_x = value;
            dpi_y = value;
        }
    }
}


static void
xresource_overrides(void)
{
    char *srs = XScreenResourceString(screen);
    char *rms = XResourceManagerString(display);
    XrmDatabase sr = srs? XrmGetStringDatabase(srs): 0;
    XrmDatabase rm = rms? XrmGetStringDatabase(rms): 0;
    XrmDatabase db = sr? sr: rm;

    if (!db)
        return;

    if (sr && rm)
        XrmCombineDatabase(rm, &sr, true);

    XrmValue string;
    if (XrmGetResource(db, "Xft.dpi", "", (char*[]) { 0 }, &string)) {
        float value = (float) strtod(string.addr, 0);

        if (value > 0.0f) {
            dpi_x = value;
            dpi_y = value;
        }
    }

    if (rm)
        XrmDestroyDatabase(rm);
    if (sr)
        XrmDestroyDatabase(sr);
}


bool
pg_init_tk(void)
{

    if (display)
        return true;

    if (!glewInit())
        return false;

    display = XOpenDisplay(0);

    if (!display)
        return false;

    WM_PROTOCOLS = XInternAtom(display, "WM_PROTOCOLS", false);
    WM_DELETE_WINDOW = XInternAtom(display, "WM_DELETE_WINDOW", false);
    RESOURCE_MANAGER = XInternAtom(display, "RESOURCE_MANAGER", false);

    screen = DefaultScreenOfDisplay(display);

    dpi_x = (float) screen->width / (float) screen->mwidth * 25.4f;
    dpi_y = (float) screen->height / (float) screen->mheight * 25.4f;

    qt_overrides();
    gdk_overrides();
    xresource_overrides();

    return true;
}


PgPt
pg_dpi(void)
{
    pg_init_tk();
    return PgPt(dpi_x, dpi_y);
}


Pg*
pg_window(unsigned width, unsigned height, const char *title)
{
    if (!pg_init_tk())
        return 0;

    GLXFBConfig     *fbcs;
    GLXFBConfig     fbc;
    XVisualInfo     *vi;
    Colormap        colormap;

    fbcs = glXChooseFBConfig(display,
                             DefaultScreen(display),
                             (int[]) {
                                GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                                GLX_RENDER_TYPE,    GLX_RGBA_BIT,
                                GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
                                GLX_DEPTH_SIZE,     24,
                                GLX_STENCIL_SIZE,   8,
                                GLX_DOUBLEBUFFER,   true,
                                GLX_SAMPLE_BUFFERS, 1,
                                GLX_SAMPLES,        8,
                                None,
                             },
                             (int[]) { 0 });

    if (!fbcs)
        return 0;

    fbc = fbcs[0];
    vi = glXGetVisualFromFBConfig(display, fbc);

    XFree(fbcs);

    if (!vi)
        return 0;

    colormap = XCreateColormap(display, screen->root, vi->visual, AllocNone);

    window = XCreateWindow(display,
                           screen->root,
                           0,
                           0,
                           width,
                           height,
                           10,
                           vi->depth,
                           InputOutput,
                           vi->visual,
                           CWColormap | CWEventMask,
                           &(XSetWindowAttributes) {
                               .colormap = colormap,
                               .event_mask = + KeyPressMask
                                             + KeyReleaseMask
                                             + ButtonPressMask
                                             + ButtonReleaseMask
                                             + PointerMotionMask
                                             + ExposureMask
                                             + StructureNotifyMask
                                             + PropertyChangeMask
                           });

    if (!window) {
        XFreeColormap(display, colormap);
        return 0;
    }

    XSetWMProtocols(display, window, &WM_DELETE_WINDOW, 1);

    if (title)
        XStoreName(display, window, title);

    XMapWindow(display, window);

    gl = glXCreateNewContext(display,
                             fbc,
                             GLX_RGBA_TYPE,
                             0,
                             true);

    XSync(display, false);

    glXMakeCurrent(display, window, gl);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g = pg_opengl((float) width, (float) height);
    return g;
}


static
int32_t
key(KeySym keysym)
{
    if ((0x20 <= keysym && keysym <= 0x7f) ||
        (0xa0 <= keysym && keysym <= 0xff) ||
        (0x100 <= keysym && keysym <= 0x20ff))
    {
        return (int32_t) keysym;
    }

    if (0x01000100 <= keysym && keysym <= 0x0110ffff)
        return (int32_t) keysym - 0x01000000;

    switch (keysym) {

    case XK_VoidSymbol:     return 0;
    case XK_BackSpace:      return PG_KEY_BACKSPACE;
    case XK_Tab:            return PG_KEY_TAB;
    case XK_Linefeed:       return PG_KEY_ENTER;
    case XK_Return:         return PG_KEY_ENTER;
    case XK_Pause:          return PG_KEY_PAUSE;
    case XK_Scroll_Lock:    return PG_KEY_SCROLL_LOCK;
    case XK_Sys_Req:        return PG_KEY_SYS_REQ;
    case XK_Escape:         return PG_KEY_ESCAPE;
    case XK_Delete:         return PG_KEY_DELETE;
    case XK_Muhenkan:       return PG_KEY_NO_CONVERT;
    case XK_Henkan:         return PG_KEY_CONVERT;
    case XK_Romaji:         return PG_KEY_KANA;
    case XK_Hiragana:       return PG_KEY_KANA;
    case XK_Katakana:       return PG_KEY_KANA;
    case XK_Hiragana_Katakana: return PG_KEY_KANA;
    case XK_Zenkaku:        return PG_KEY_HANKAKU;
    case XK_Hankaku:        return PG_KEY_HANKAKU;
    case XK_Zenkaku_Hankaku: return PG_KEY_HANKAKU;
    case XK_Hangul:         return PG_KEY_HANGUL;
    case XK_Hangul_Hanja:   return PG_KEY_HANJA;
    case XK_Home:           return PG_KEY_HOME;
    case XK_Left:           return PG_KEY_LEFT;
    case XK_Up:             return PG_KEY_UP;
    case XK_Right:          return PG_KEY_RIGHT;
    case XK_Down:           return PG_KEY_DOWN;
    case XK_Prior:          return PG_KEY_PAGE_UP;
    case XK_Next:           return PG_KEY_PAGE_DOWN;
    case XK_End:            return PG_KEY_END;
    case XK_Print:          return PG_KEY_PRINT_SCREEN;
    case XK_Execute:        return PG_KEY_EXECUTE;
    case XK_Insert:         return PG_KEY_INSERT;
    case XK_Undo:           return PG_KEY_UNDO;
    case XK_Redo:           return PG_KEY_REDO;
    case XK_Menu:           return PG_KEY_MENU;
    case XK_Find:           return PG_KEY_FIND;
    case XK_Cancel:         return PG_KEY_STOP;
    case XK_Help:           return PG_KEY_HELP;
    case XK_Num_Lock:       return PG_KEY_NUM_LOCK;
    case XK_KP_Enter:       return PG_KEY_KP_ENTER;
    case XK_KP_Home:        return PG_KEY_KP_7;
    case XK_KP_Left:        return PG_KEY_KP_4;
    case XK_KP_Up:          return PG_KEY_KP_8;
    case XK_KP_Right:       return PG_KEY_KP_6;
    case XK_KP_Down:        return PG_KEY_KP_2;
    case XK_KP_Prior:       return PG_KEY_KP_9;
    case XK_KP_Next:        return PG_KEY_KP_3;
    case XK_KP_End:         return PG_KEY_KP_1;
    case XK_KP_Equal:       return PG_KEY_KP_EQUAL;
    case XK_KP_Multiply:    return PG_KEY_KP_MULTIPLY;
    case XK_KP_Add:         return PG_KEY_KP_ADD;
    case XK_KP_Separator:   return PG_KEY_KP_SEPARATOR;
    case XK_KP_Subtract:    return PG_KEY_KP_SUBTRACT;
    case XK_KP_Decimal:     return PG_KEY_KP_SUBTRACT;
    case XK_KP_Divide:      return PG_KEY_KP_DIVIDE;
    case XK_KP_0:           return PG_KEY_KP_0;
    case XK_KP_1:           return PG_KEY_KP_1;
    case XK_KP_2:           return PG_KEY_KP_2;
    case XK_KP_3:           return PG_KEY_KP_3;
    case XK_KP_4:           return PG_KEY_KP_4;
    case XK_KP_5:           return PG_KEY_KP_5;
    case XK_KP_6:           return PG_KEY_KP_6;
    case XK_KP_7:           return PG_KEY_KP_7;
    case XK_KP_8:           return PG_KEY_KP_8;
    case XK_KP_9:           return PG_KEY_KP_9;
    case XK_F1:             return PG_KEY_F1;
    case XK_F2:             return PG_KEY_F2;
    case XK_F3:             return PG_KEY_F3;
    case XK_F4:             return PG_KEY_F4;
    case XK_F5:             return PG_KEY_F5;
    case XK_F6:             return PG_KEY_F6;
    case XK_F7:             return PG_KEY_F7;
    case XK_F8:             return PG_KEY_F8;
    case XK_F9:             return PG_KEY_F9;
    case XK_F10:            return PG_KEY_F10;
    case XK_F11:            return PG_KEY_F11;
    case XK_F12:            return PG_KEY_F12;
    case XK_F13:            return PG_KEY_F13;
    case XK_F14:            return PG_KEY_F14;
    case XK_F15:            return PG_KEY_F15;
    case XK_F16:            return PG_KEY_F16;
    case XK_F17:            return PG_KEY_F17;
    case XK_F18:            return PG_KEY_F18;
    case XK_F19:            return PG_KEY_F19;
    case XK_F20:            return PG_KEY_F20;
    case XK_F21:            return PG_KEY_F21;
    case XK_F22:            return PG_KEY_F22;
    case XK_F23:            return PG_KEY_F23;
    case XK_F24:            return PG_KEY_F24;
    case XK_Shift_L:        return PG_KEY_SHIFT_LEFT;
    case XK_Shift_R:        return PG_KEY_SHIFT_RIGHT;
    case XK_Control_L:      return PG_KEY_CTRL_LEFT;
    case XK_Control_R:      return PG_KEY_CTRL_RIGHT;
    case XK_Alt_L:          return PG_KEY_ALT_LEFT;
    case XK_Alt_R:          return PG_KEY_ALT_RIGHT;
    case XK_Super_L:        return PG_KEY_WIN_LEFT;
    case XK_Super_R:        return PG_KEY_WIN_RIGHT;
    case XK_Caps_Lock:      return PG_KEY_CAPS_LOCK;
    }

    return (int32_t) -keysym;
}


static
unsigned
modifiers(unsigned state)
{
    return + (state & ShiftMask ? PG_MOD_SHIFT: 0)
           + (state & ControlMask ? PG_MOD_CTRL: 0)
           + (state & Mod1Mask ? PG_MOD_ALT: 0)
           + (state & Mod4Mask ? PG_MOD_WIN: 0);
}


static
unsigned
buttons(unsigned state)
{
    return  + (state & Button1Mask? 1: 0)
            + (state & Button2Mask? 2: 0)
            + (state & Button3Mask? 4: 0);
}


bool
pg_wait(void)
{
    if (redraw) {
        redraw = false;

        if (!XPending(display))
            return true;
    }

    XEvent e;

    XNextEvent(display, &e);

    switch (e.type & ~128) {

    case Expose:
        break;

    case MappingNotify:
        XRefreshKeyboardMapping(&e.xmapping);
        break;

    case KeyRelease:
        _key = 0;
        _mods = modifiers(e.xkey.state);
        break;

    case KeyPress:

        {
            bool    caps = e.xkey.state & LockMask;
            bool    shift = e.xkey.state & ShiftMask;
            int     index = shift ^ caps;
            KeySym  keysym = XLookupKeysym(&e.xkey, index);
            _key = key(keysym);
            _mods = modifiers(e.xkey.state);
        }

        break;

    case MotionNotify:
        {
            // The X Server sends too many reports.
            // Only look at the last.
            while (XCheckTypedWindowEvent(display, window, MotionNotify, &e)) {
            }

            _mouse_x = (float) e.xmotion.x;
            _mouse_y = (float) e.xmotion.y;
            _mods = modifiers(e.xmotion.state);
            _buttons = buttons(e.xmotion.state);
            break;
        }

    case ButtonPress:
    case ButtonRelease:
        {
            _mouse_x = (float) e.xbutton.x;
            _mouse_y = (float) e.xbutton.y;
            _mods = modifiers(e.xbutton.state);

            if (e.xbutton.button == 4)
                _wheel = e.type == ButtonPress? -1.0f: .0f;
            else if (e.xbutton.button == 5)
                _wheel = e.type == ButtonPress? +1.0f: .0f;
            else
                _buttons = buttons(e.xbutton.state) +
                            (e.type == ButtonPress
                             ? 1 << (e.xbutton.button - 1)
                             : -(1 << (e.xbutton.button - 1)));
            break;
        }

    case ConfigureNotify:

        if ((float) e.xconfigure.width != g->sx ||
            (float) e.xconfigure.height != g->sy)
        {
            pg_resize(g,
                      (float) e.xconfigure.width,
                      (float) e.xconfigure.height);
        }
        break;

    case CreateNotify:
        break;

    case DestroyNotify:
        return false;

    case ClientMessage:

        if (e.xclient.message_type == WM_PROTOCOLS &&
            e.xclient.data.l[0] == (long) WM_DELETE_WINDOW)
        {
            XDestroyWindow(display, window);
            return false;
        }
        break;

    }
    return true;

}


void
pg_update(void)
{
    glXSwapBuffers(display, window);
}


void
pg_redraw(void)
{
    redraw = true;
}


Pg*
pg_root_canvas(void)
{
    return g;
}


PgPt
pg_mouse_at(void)
{
    return PgPt(_mouse_x, _mouse_y);
}


unsigned
pg_mouse_buttons(void)
{
    return _buttons;
}


float
pg_mouse_wheel(void)
{
    return _wheel;
}


int32_t
pg_key(void)
{
    return _key;
}


void
pg_set_key(int32_t key)
{
    _key = key;
}


unsigned
pg_mod_keys(void)
{
    return _mods;
}

#endif
