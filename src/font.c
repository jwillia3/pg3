#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <pg.h>
#include <pg-utf-8.h>
#include <pg-internal-font.h>
#include <pg-internal-platform.h>


static PgFamily *_families;
static unsigned _nfamilies;


static int
compare_face(const void *ap, const void *bp)
{
    const PgFace    *a = (const PgFace*) ap;
    const PgFace    *b = (const PgFace*) bp;
    int             r;

    return (r = pg_stricmp(a->family, b->family))? r:
            a->width_class < b->width_class?    -1:
            a->width_class > b->width_class?    +1:

            a->weight < b->weight?  -1:
            a->weight > b->weight?  +1:

            a->is_italic && !b->is_italic?  -1:
            !a->is_italic && b->is_italic?  +1:

            pg_stricmp(a->style, b->style);
}


void
pg_font_list_free(void)
{
    if (!_families)
        return;

    for (PgFamily *fam = _families; fam->name; fam++) {

        free((void*) fam->faces[0].family);

        for (PgFace *face = fam->faces; face->family; face++) {
            free((void*) face->style);
            free((void*) face->path);
        }
    }

    free(_families);
    _families = 0;
    _nfamilies = 0;
}


unsigned
pg_font_list_get_count(void) {
    pg_font_list();
    return _nfamilies;
}


static
char **
get_font_files(void)
{
    char        path[PATH_MAX];
    char        *queue[256];
    unsigned    nqueue = 0;
    unsigned    max = sizeof queue / sizeof *queue;
    char        **files = 0;
    unsigned    nfiles = 0;

    // Get roots.
    nqueue = _pg_fontconfig_font_dirs(queue, max);
    if (nqueue == 0)
        nqueue = _pg_default_font_dirs(queue, max);

    // Recursively list files in directories.
    while (nqueue) {
        char            *dirname = queue[--nqueue];
        DIR             *dir = opendir(dirname);
        struct dirent   *e;

        while (dir && (e = readdir(dir))) {
            struct stat st;

            if (snprintf(path, PATH_MAX, "%s/%s", dirname, e->d_name) >= PATH_MAX)
                continue;

            if (e->d_name[0] == '.') {
                // Ignore hidden files.
            }
            else if (stat(path, &st) >= 0 && S_ISDIR(st.st_mode)) {
                /*
                    Queue directories.
                    We can do a cursory search of the remaining queue
                    to prevent duplication, but we don't keep track of
                    duplicates we've already processed and removed from
                    the queue.
                 */
                if (nqueue < max) {
                    char    **dup = queue;
                    char    **end = dup + nqueue;
                    while (dup < end && strcmp(*dup, path)) dup++;
                    if (dup == end)
                        queue[nqueue++] = strdup(path);
                }
            }
            else {
                // Add files to the list.
                char *ext = strrchr(path, '.');
                static char *extensions[] = {".ttf", ".ttc", ".otf", 0};

                for (char **i = extensions; ext && *i; i++)
                    if (!pg_stricmp(ext, *i)) {
                        files = realloc(files, (nfiles + 2) * sizeof *files);
                        files[nfiles++] = strdup(path);
                        break;
                    }
            }
        }

        free(dirname);
    }

    if (nfiles)
        files[nfiles] = 0;

    return files;
}


const PgFamily*
pg_font_list(void)
{

    if (_families)
        return _families;

    char **files = get_font_files();

    if (!files)
        return calloc(1, sizeof(PgFamily));

    // Get font properties.
    PgFace      *faces = 0;
    unsigned    nfaces = 0;

    for (unsigned i = 0; files[i]; i++) {
        PgFont  *font = pg_font_from_file(files[i], 0);

        if (font) {
            PgFace f = {0};

            f.family = strdup(pg_font_prop_string(font, PG_FONT_FAMILY));
            f.style = strdup(pg_font_prop_string(font, PG_FONT_STYLE));
            f.path = files[i];
            f.index = (unsigned) pg_font_prop_int(font, PG_FONT_INDEX);
            f.width_class = (unsigned) pg_font_prop_int(font, PG_FONT_WIDTH_CLASS);
            f.weight = (unsigned) pg_font_prop_int(font, PG_FONT_WEIGHT);
            f.is_fixed = (unsigned) pg_font_prop_int(font, PG_FONT_IS_FIXED);
            f.is_italic = pg_font_prop_int(font, PG_FONT_IS_ITALIC);
            f.is_serif = (unsigned) pg_font_prop_int(font, PG_FONT_IS_SERIF);
            f.is_sans_serif = (unsigned) pg_font_prop_int(font, PG_FONT_IS_SANS_SERIF);
            f.style_class = (uint8_t) pg_font_prop_int(font, PG_FONT_STYLE_CLASS);
            f.style_subclass = (uint8_t) pg_font_prop_int(font, PG_FONT_STYLE_SUBCLASS);

            for (unsigned i = 0; i < 10; i++)
                f.panose[i] = (uint8_t) pg_font_prop_int(font, PG_FONT_PANOSE_1 + i);

            pg_font_free(font);

            faces = realloc(faces, (nfaces + 2) * sizeof *faces);
            faces[nfaces++] = f;
        }
    }

    if (!faces)
        faces = malloc(sizeof *faces);

    faces[nfaces] = (PgFace) { 0 };
    free(files);

    // Sort them and put them into families.
    qsort(faces, nfaces, sizeof *faces, compare_face);

    // Group into families.
    PgFamily    *families = 0;
    unsigned    nfamilies = 0;
    for (unsigned i = 0; i < nfaces; ) {
        unsigned first = i;
        while (i < nfaces && !strcmp(faces[i].family, faces[first].family))
            i++;

        unsigned    n = i - first;
        PgFace      *children = malloc((n + 1) * sizeof *faces);
        memcpy(children, faces + first, n * sizeof *faces);
        children[n] = (PgFace) { 0 };

        families = realloc(families, (nfamilies + 2) * sizeof *families);
        families[nfamilies++] = (PgFamily) {faces[first].family, n, children};
    }

    if (!families)
        families = malloc(sizeof *families);

    families[nfamilies] = (PgFamily) { 0 };
    free(faces);

    _families = families;
    _nfamilies = nfamilies;
    return families;
}


static
bool
exact_family_name_exists(const char *family)
{
    for (const PgFamily *fam = pg_font_list(); fam->name; fam++)
        if (!pg_stricmp(fam->name, family)) return true;
    return false;
}


static
const char*
fallback_substitute(const char *family)
{
    static const char *sans_options[] = {
        "Segoe UI",
        "Arial",
        "SF Pro"
        "Helveticas",
        "Roboto",
        "FreeSans",
        "Liberation Sans",
        "Nimbus Sans",
        "DejaVu Sans",
        "Open Sans",
        NULL,
    };
    static const char *serif_options[] = {
        "Sitka",
        "Linux Libertine",
        "Liberation Serif",
        "Nimbus Roman",
        "DejaVu Serif",
        "Nimbus Sans",
        "Times New Roman",
        NULL,
    };
    static const char *mono_options[] = {
        "Courier Prime",
        "Consolas",
        "Cascadia Code",
        "SF Mono",
        "Liberation Mono",
        "Nimbus Mono",
        "Nimbus Mono PS",
        "DejaVu Sans Mono",
        "Nimbus Mono",
        "Courier New",
        NULL,
    };
    static const char *no_options[] = { NULL };


    if (exact_family_name_exists(family))
        return 0;

    if (!pg_stricmp(family, "any") || !pg_stricmp(family, "default")) {
        /*
            This is the last ditch effort.
            If no other option matches, this will return any
            font on the system.
         */
        const PgFamily *all = pg_font_list();
        return all? all->name: NULL;
    }


    const char **options =
        !pg_stricmp(family, "sans")?           sans_options:
        !pg_stricmp(family, "sans-serif")?     sans_options:
        !pg_stricmp(family, "system-ui")?      sans_options:
        !pg_stricmp(family, "ui-sans-serif")?  sans_options:
        !pg_stricmp(family, "serif")?          serif_options:
        !pg_stricmp(family, "ui-serif")?       serif_options:
        !pg_stricmp(family, "monospace")?      mono_options:
        !pg_stricmp(family, "ui-monospace")?   mono_options:
        no_options;

    for (const char **i = options; *i; i++)
        if (exact_family_name_exists(*i))
            return strdup(*i);

    return NULL;
}




static
PgFont*
find_single_font(const char *family, unsigned weight, bool italic)
{
    if (!family)
        return 0;

    // Search for family name in the list.
    const PgFamily *fam = pg_font_list();
    while (fam->name && pg_stricmp(fam->name, family))
        fam++;

    const char *substitute;;

    if ((substitute = _pg_fontconfig_substitute(family)) ||
        (substitute = fallback_substitute(family)))
    {
        const PgFamily *search = pg_font_list();
        while (search->name && pg_stricmp(search->name, substitute))
            search++;
        if (search->name)
            fam = search;
        free((void*) substitute);
        substitute = 0;
    }

    if (!fam->name)
        return 0;

    // Default to Normal weight.
    if (weight == 0)
        weight = 400;

    // Score each option and try to find the best.
    PgFace  *best = fam->faces;
    int     bestscore = INT_MIN;

    for (PgFace *face = fam->faces; face->family; face++) {
        int score =
            -abs((int) 5 - (int) face->width_class)    * 1000
            -abs((int) weight - (int) face->weight)    * 1
            -abs((int) italic - (int) face->is_italic) * 101
            + 0;

        if (score > bestscore) {
            best = face;
            bestscore = score;
        }
    }

    return pg_font_from_file(best->path, best->index);
}


PgFont*
pg_font_find(const char *family_options, unsigned weight, bool italic)
{
    if (!family_options)
        return 0;

    char    *options = strdup(family_options);
    char    *prev = 0;
    char    *family;
    PgFont  *font = 0;
    for (family = strtok_r(options, ",", &prev);
         !font && family;
         family = strtok_r(0, ",", &prev))
    {
        while (*family && isspace(*family)) family++;
        font = find_single_font(family, weight, italic);
    }
    free(options);
    return font;
}


PgFont*
pg_font_from_data(const char *optional_path, const uint8_t *data, size_t size, unsigned index)
{
    if (!data)
        return 0;

    if (!size)
        return 0;

    PgFont *font;

    if ((font = pg_font_from_data_otf(data, size, index))) {
        if (optional_path)
            font->path = strdup(optional_path);
        return font;
    }

    return 0;
}


PgFont*
pg_font_from_file(const char *path, unsigned index)
{
    if (!path)
        return 0;

    size_t  size = 0;
    void    *data = _pg_file_map(path, &size);
    PgFont  *font = pg_font_from_data(path, data, size, index);

    if (!font) {
        _pg_file_unmap(data, size);
        return 0;
    }

    return font;
}


PgFont
_pg_font_init(const PgFontFunc *v,
             const uint8_t *data,
             size_t size,
             unsigned index,
             unsigned nglyphs,
             float units,
             float ascender,
             float descender)
{
    return (PgFont) {
        .v = v,
        .data = data,
        .size = size,
        .index = index,
        .sx = 1.0f,
        .sy = 1.0f,
        .nglyphs = nglyphs,
        .units = units,
        .ascender = ascender,
        .descender = descender,
        .prop_buf = NULL,
    };
}


void
pg_font_free(PgFont *font)
{
    if (!font)
        return;

    if (!font->v && !font->v->free)
        return;

    font->v->free(font);
    _pg_file_unmap((void*) font->data, font->size);
    free(font->prop_buf);
    free((void*) font->path);
    free(font);
}


PgFont*
pg_font_clone(PgFont *font)
{
    if (!font || !font->v || !font->v->clone)
        return NULL;

    PgFont  *clone = font->v->clone(font);

    if (!clone)
        return NULL;

    clone->prop_buf = NULL;
    return clone;
}


PgPt
pg_font_get_scale(PgFont *font)
{
    return font? pgpt(font->sx, font->sy): pgpt(0, 0);
}


PgPt
pg_font_get_em(PgFont *font)
{
    return font? pgpt(font->units * font->sx, font->units * font->sy): pgpt(0, 0);
}


float
pg_font_prop_float(PgFont *font, PgFontProp id)
{
    if (!font)
        return 0.0f;

    if (!font->v && !font->v->number)
        return 0.0f;

    return font->v->number(font, id);
}


int
pg_font_prop_int(PgFont *font, PgFontProp id)
{
    if (!font)
        return 0.0f;

    if (!font->v && !font->v->_int)
        return 0.0f;

    return font->v->_int(font, id);
}


const char*
pg_font_prop_string(PgFont *font, PgFontProp id)
{
    if (!font)
        return "";

    if (!font->v || !font->v->string)
        return "";

    if (!font->prop_buf)
        font->prop_buf = malloc(256);

    return font->v->string(font, id);
}


const char*
pg_font_get_path(PgFont *font)
{
    return font? font->path: NULL;
}


PgFont*
pg_font_scale(PgFont *font, float sx, float sy)
{
    if (!font)
        return font;

    if (!font->v || !font->v->scale)
        return font;

    if (sx == 0.0f && sy == 0.0f)
        return font;

    if (font->units == 0.0f)
        return font;

    // Fill in sy or sx if the other is set.
    if (sx == 0.0f)
        sx = sy;
    else if (sy == 0.0f)
        sy = sx;

    font->sx = sx / font->units;
    font->sy = sy / font->units;

    font->v->scale(font, sx, sy);
    return font;
}


float
pg_font_get_height(PgFont *font)
{
    return pg_font_get_em(font).y;
}


unsigned
pg_font_char_to_glyph(PgFont *font, uint32_t codepoint)
{
    if (!font)
        return 0;

    if (!font->v || !font->v->get_glyph)
        return 0;

    return font->v->get_glyph(font, codepoint);
}


float
pg_canvas_trace_glyph(Pg *g, PgFont *font, float x, float y, unsigned glyph)
{
    if (!g)
        return 0.0f;

    if (!font)
        return 0.0f;

    if (!font->v && !font->v->glyph_path)
        return 0.0f;

    if (glyph >= font->nglyphs)
        return 0.0f;

    font->v->glyph_path(g, font, x, y, glyph);
    return x + pg_font_measure_glyph(font, glyph);
}


float
pg_canvas_trace_char(Pg *g, PgFont *font, float x, float y, uint32_t codepoint)
{
    return pg_canvas_trace_glyph(g, font, x, y, pg_font_char_to_glyph(font, codepoint));
}


float
pg_canvas_trace_chars(Pg *g, PgFont *font, float x, float y, const char *str, size_t nbytes)
{
    if (!g)
        return 0.0f;

    if (!font)
        return 0.0f;

    if (!str)
        return 0.0f;


    const char  *i = str;
    const char  *end = i + nbytes;
    float       xi = x;

    while (i < end)
        xi = pg_canvas_trace_char(g, font, xi, y, pg_read_utf8(&i, end));

    return xi;
}


float
pg_canvas_trace_string(Pg *g, PgFont *font, float x, float y, const char *str)
{
    if (!str)
        return 0.0f;

    return pg_canvas_trace_chars(g, font, x, y, str, strlen(str));
}


float
pg_canvas_show_char(Pg *g, PgFont *font, float x, float y, uint32_t codepoint)
{
    if (!g || !font)
        return x;
    float out_x = pg_canvas_trace_char(g, font, x, y, codepoint);
    pg_canvas_fill(g);
    return out_x;
}


float
pg_canvas_show_chars(Pg *g, PgFont *font, float x, float y, const char *str, size_t nbytes)
{
    if (!g || !font)
        return x;
    float out_x = pg_canvas_trace_chars(g, font, x, y, str, nbytes);
    pg_canvas_fill(g);
    return out_x;
}


float
pg_canvas_show_string(Pg *g, PgFont *font, float x, float y, const char *str)
{
    if (!g || !font)
        return x;
    float out_x = pg_canvas_trace_string(g, font, x, y, str);
    pg_canvas_fill(g);
    return out_x;
}


float
pg_canvas_show_glyph(Pg *g, PgFont *font, float x, float y, uint32_t glyph)
{
    if (!g || !font)
        return x;
    float out_x = pg_canvas_trace_glyph(g, font, x, y, glyph);
    pg_canvas_fill(g);
    return out_x;
}


float
pg_canvas_vprintf(Pg *g, PgFont *font, float x, float y, const char *str, va_list ap)
{
    if (!g)
        return x;

    if (!font)
        return x;

    if (!str)
        return x;

    char        small[1024];
    size_t      size;
    char        *buf = small;
    bool        allocate = false;
    float       max_x = x;


    // Attempt to write in small buffer.
    va_list tmp;
    va_copy(tmp, ap);
    size = (size_t) vsnprintf(small, sizeof small, str, tmp);
    va_end(tmp);

    // If it didn't fit, allocate a buffer.
    if (size + 1 >= sizeof small) {
        allocate = true;
        buf = malloc(size + 1);
        snprintf(buf, size + 1, str, ap);
    }

    // Output one line at a time.
    while (true) {

        size_t line_len = strcspn(buf, "\n");

        float new_x = pg_canvas_show_chars(g, font, x, y, buf, line_len);

        max_x = fmaxf(max_x, new_x);
        y += pg_font_get_height(font);

        if (!buf[line_len])
            break;

        buf += line_len + 1;
    }

    if (allocate)
        free(buf);

    return max_x;
}


float
pg_canvas_printf(Pg *g, PgFont *font, float x, float y, const char *str, ...)
{
    if (!font)
        return x;
    va_list ap;

    va_start(ap, str);
    float new_x = pg_canvas_vprintf(g, font, x, y, str, ap);
    va_end(ap);

    return new_x;
}


float
pg_font_measure_glyph(PgFont *font, unsigned glyph)
{
    if (!font)
        return 0.0f;

    if (!font->v || !font->v->measure_glyph)
        return 0.0f;

    if (glyph >= font->nglyphs)
        return 0.0f;

    return font->v->measure_glyph(font, glyph);
}


float
pg_font_measure_char(PgFont *font, uint32_t codepoint)
{
    return pg_font_measure_glyph(font, pg_font_char_to_glyph(font, codepoint));
}


float
pg_font_measure_chars(PgFont *font, const char *str, size_t nbytes)
{
    if (!font)
        return 0.0f;

    if (!str)
        return 0.0f;

    if (!nbytes)
        return 0.0f; // Not an error but exit early.


    const char  *i = str;
    const char  *end = i + nbytes;
    float       width = 0.0f;

    while (i < end)
        width += pg_font_measure_char(font, pg_read_utf8(&i, end));

    return width;
}


float
pg_font_measure_string(PgFont *font, const char *str)
{
    if (!font)
        return 0.0f;

    if (!str)
        return 0.0f;

    return pg_font_measure_chars(font, str, strlen(str));
}


unsigned
pg_font_fit_chars(PgFont *font, const char *s, size_t nbytes, float width)
{
    if (font && s) {
        float       total = 0.0f;
        unsigned    nfit = 0;
        const char  *i = s;
        const char  *end = i + nbytes;

        while (i < end) {
            float size = pg_font_measure_char(font, pg_read_utf8(&i, end));

            if (total + size > width)
                break;
            total += size;
            nfit++;
        }
        return nfit;
    }
    return 0;
}


unsigned
pg_font_fit_string(PgFont *font, const char *str, float width)
{
    if (!font)
        return 0;

    if (!str)
        return 0;

    return pg_font_fit_chars(font, str, strlen(str), width);
}


const PgFamily*
pg_font_list_get_family(unsigned n)
{
    return n < pg_font_list_get_count()? _families + n: NULL;
}


const char*
pg_font_family_get_name(const PgFamily *family)
{
    return family? family->name: NULL;
}


const PgFace*
pg_font_family_get_face(const PgFamily *family, unsigned n)
{
    return family && n < family->nfaces? family->faces + n: NULL;
}


unsigned
pg_font_family_get_face_count(const PgFamily *family)
{
    return family? family->nfaces: 0;
}


const char*
pg_font_face_get_family(const PgFace *face)
{
    return face? face->family: NULL;
}


const char*
pg_font_face_get_style(const PgFace *face)
{
    return face? face->style: NULL;
}


const char*
pg_font_face_get_path(const PgFace *face)
{
    return face? face->path: NULL;
}


unsigned
pg_font_face_get_index(const PgFace *face)
{
    return face? face->index: 0;
}


unsigned
pg_font_face_get_width_class(const PgFace *face)
{
    return face? face->width_class: 0;
}


unsigned
pg_font_face_get_weight(const PgFace *face)
{
    return face? face->weight: 0;
}


bool
pg_font_face_get_is_fixed(const PgFace *face)
{
    return face? face->is_fixed: false;
}


bool
pg_font_face_get_is_italic(const PgFace *face)
{
    return face? face->is_italic: false;
}


bool
pg_font_face_get_is_serif(const PgFace *face)
{
    return face? face->is_serif: false;
}


bool
pg_font_face_get_is_sans_serif(const PgFace *face)
{
    return face? face->is_sans_serif: false;
}


unsigned
pg_font_face_get_style_class(const PgFace *face)
{
    return face? face->style_class: 0;
}


unsigned
pg_font_face_get_style_subclass(const PgFace *face)
{
    return face? face->style_subclass: 0;
}

