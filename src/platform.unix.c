#ifdef USE_UNIX


#include <ctype.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <pg3/pg.h>


static char*
env_path(char path[PATH_MAX], char *env, char *rest)
{
    char    *root = getenv(env);
    if (root && snprintf(path, PATH_MAX, "%s/%s", root, rest) < PATH_MAX)
        return path;
    return 0;
}


unsigned
_pg_default_font_dirs(char **queue, unsigned max)
{
    char        path[PATH_MAX + 1];
    unsigned    n = 0;

    if (n < max)
        queue[n++] = strdup("/usr/share/fonts");

    if (n < max)
        queue[n++] = strdup("/usr/local/share/fonts");

    if (n < max && env_path(path, "HOME", ".fonts"))
        queue[n++] = strdup(path);

    if (n < max &&
        (env_path(path, "XDG_DATA_HOME", "fonts") ||
         env_path(path, "HOME", ".local/share/fonts")))
    {
        queue[n++] = strdup(path);
    }

    return n;
}


void*
_pg_file_map(const char *path, size_t *size)
{
    if (!path)
        return 0;

    struct stat st;
    int         fd = open(path, O_RDONLY);

    if (fd < 0)
        return 0;

    if (fstat(fd, &st) < 0)
        return 0;

    *size = (size_t) st.st_size;

    void *data = mmap(0, *size, PROT_READ, MAP_PRIVATE, fd, 0);

    close(fd);

    if (data == MAP_FAILED)
        return 0;

    return data;
}


void
_pg_file_unmap(void *ptr, size_t size)
{
    munmap(ptr, size);
}


static
PgPt
xrdb_dpi(void)
{
    typedef void    *XrmDatabase, Display;
    typedef struct XrmValue {
        unsigned size;
        void     *addr;
    } XrmValue;

    void        *xlib;
    Display*    (*XOpenDisplay)(const char *display_name);
    int         (*XCloseDisplay)(Display *display);
    void        (*XrmInitialize)(void);
    char*       (*XResourceManagerString)(Display *display);
    XrmDatabase (*XrmGetStringDatabase)(const char *string);
    void        (*XrmDestroyDatabase)(XrmDatabase *database);
    bool        (*XrmGetResource)(XrmDatabase database, char *str_name, char *str_class,
                        char **str_type_return, XrmValue *value_return);

    if (!(xlib = dlopen("libX11.so", RTLD_LAZY)) ||
        !(XOpenDisplay = dlsym(xlib, "XOpenDisplay")) ||
        !(XCloseDisplay = dlsym(xlib, "XCloseDisplay")) ||
        !(XrmInitialize = dlsym(xlib, "XrmInitialize")) ||
        !(XResourceManagerString = dlsym(xlib, "XResourceManagerString")) ||
        !(XrmGetStringDatabase = dlsym(xlib, "XrmGetStringDatabase")) ||
        !(XrmDestroyDatabase = dlsym(xlib, "XrmDestroyDatabase")) ||
        !(XrmGetResource = dlsym(xlib, "XrmGetResource")))
    {
        if (xlib) dlclose(xlib);
        return pgpt(0,0);
    }

    Display         *display = 0;
    char            *dbstr, *type, *suffix;
    XrmDatabase     rmdb = 0;
    XrmValue        value;
    double          dpi;
    bool            ok =
        (display = XOpenDisplay(0)) &&
        (XrmInitialize(), true) &&
        (dbstr = XResourceManagerString(display)) &&
        (rmdb = XrmGetStringDatabase(dbstr)) &&
        XrmGetResource(rmdb, "Xft.dpi", "String", &type, &value) &&
        !strcmp(type, "String") &&
        (dpi = strtod(value.addr, &suffix)) &&
        !*suffix;

    if (rmdb) XrmDestroyDatabase(rmdb);
    if (display) XCloseDisplay(display);
    dlclose(xlib);

    if (ok)
        return pgpt(dpi, dpi);

    return pgpt(0, 0);
}


static
PgPt
qt_dpi(void)
{
    char    *txt;
    float   dpi = 0;

    if ((txt = getenv("QT_AUTO_SCREEN_SCALE_FACTOR")) ||
        (txt = getenv("QT_SCALE_FACTOR")))
        dpi = strtod(txt, 0) * 96;

    if (!dpi && (txt = getenv("QT_FONT_DPI")))
        dpi = strtod(txt, 0);

    return pgpt(dpi, dpi);
}

static
PgPt
gdk_dpi(void)
{
    char    *txt;
    float   dpi = 0;

    if ((txt = getenv("GDK_SCALE")) ||
        (txt = getenv("GDK_DPI_SCALE")))
        dpi = strtod(txt, 0) * 96;

    return pgpt(dpi, dpi);
}


PgPt
_pg_window_get_dpi_platform(PgWindow *win)
{
    (void) win;

    PgPt    dpi;

    if ((dpi = xrdb_dpi()).x ||
        (dpi = gdk_dpi()).x ||
        (dpi = qt_dpi()).x)
        return dpi;

    return pgpt(0, 0);
}


#endif
