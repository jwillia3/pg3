#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg3.h>
#include <internal.h>


static PgFamily *_families;
static unsigned _nfamilies;

static int
compare_face(const void *ap, const void *bp)
{
    const PgFace    *a = (const PgFace*) ap;
    const PgFace    *b = (const PgFace*) bp;
    int             r;

    return (r = stricmp(a->family, b->family))? r:
            a->width < b->width?    -1:
            a->width > b->width?    +1:

            a->weight < b->weight?  -1:
            a->weight > b->weight?  +1:

            a->is_italic && !b->is_italic?  -1:
            !a->is_italic && b->is_italic?  +1:

            stricmp(a->style, b->style);
}

void
_pg_free_font_list(void)
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
}


unsigned
pg_get_family_count(void) {
    pg_list_fonts();
    return _nfamilies;
}


PgFamily*
pg_list_fonts(void)
{

    if (_families)
        return _families;

    char **files = _pgget_font_files();

    if (!files)
        return calloc(1, sizeof(PgFamily));

    // Get font properties.
    PgFace      *faces = 0;
    unsigned    nfaces = 0;

    for (unsigned i = 0; files[i]; i++) {
        PgFont  *font = pg_open_font_file(files[i], 0);

        if (font) {
            PgFace f = {0};

            f.family = strdup(pg_font_string(font, PG_FONT_FAMILY));
            f.style = strdup(pg_font_string(font, PG_FONT_STYLE));
            f.path = files[i];
            f.index = (unsigned) pg_font_int(font, PG_FONT_INDEX);
            f.width = (unsigned) pg_font_int(font, PG_FONT_WIDTH_CLASS);
            f.weight = (unsigned) pg_font_int(font, PG_FONT_WEIGHT);
            f.is_fixed = (unsigned) pg_font_int(font, PG_FONT_IS_FIXED);
            f.is_italic = pg_font_int(font, PG_FONT_IS_ITALIC);
            f.is_serif = (unsigned) pg_font_int(font, PG_FONT_IS_SERIF);
            f.is_sans_serif = (unsigned) pg_font_int(font, PG_FONT_IS_SANS_SERIF);
            f.style_class = (uint8_t) pg_font_int(font, PG_FONT_STYLE_CLASS);
            f.style_subclass = (uint8_t) pg_font_int(font, PG_FONT_STYLE_SUBCLASS);

            for (unsigned i = 0; i < 10; i++)
                f.panose[i] = (uint8_t) pg_font_int(font, PG_FONT_PANOSE_1 + i);

            pg_free_font(font);

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
PgFont*
find_single_font(const char *family, unsigned weight, bool italic)
{
    if (!family)
        return 0;

    // Search for family name in the list.
    PgFamily *fam = pg_list_fonts();
    while (fam->name && stricmp(fam->name, family))
        fam++;

    // Ask the host system for replacement advice.
    const char *replacement = _pg_advise_family_replace(family);
    if (replacement) {
        PgFamily *search = pg_list_fonts();
        while (search->name && stricmp(search->name, replacement))
            search++;
        if (search->name)
            fam = search;
        free((void*) replacement);
        replacement = 0;
    }

    // Last resort: use the first font in the list.
    if (!stricmp(family, "any") || !stricmp(family, "default"))
        fam = pg_list_fonts();

    if (!fam->name)
        return 0;

    // Default to Normal weight.
    if (weight == 0)
        weight = 400;

    // Score each option and try to find the best.
    PgFace  *best = fam->faces;
    int     bestscore = -1000;

    for (PgFace *face = fam->faces; face->family; face++) {
        int score =
            -abs((int) 5 - (int) face->width)             * 10
            -abs((int) weight - (int) face->weight) / 100 * 3
            -abs((int) italic - (int) face->is_italic)    * 2
            + 0;

        if (score > bestscore) {
            best = face;
            bestscore = score;
        }
    }

    return pg_open_font_file(best->path, best->index);
}


PgFont*
pg_find_font(const char *family_options, unsigned weight, bool italic)
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
pg_open_font(const uint8_t *data, size_t size, unsigned index)
{
    if (!data)
        return 0;

    if (!size)
        return 0;

    PgFont *font;

    if ((font = pg_open_otf_font(data, size, index)))
        return font;

    return 0;
}


PgFont*
pg_open_font_file(const char *path, unsigned index)
{
    if (!path)
        return 0;

    size_t  size = 0;
    void    *data = _pgmap_file(path, &size);
    PgFont  *font = pg_open_font(data, size, index);

    if (!font) {
        _pgunmap_file(data, size);
        return 0;
    }

    return font;
}


PgFont
pg_init_font(const PgFontImpl *v, const uint8_t *data, size_t size, unsigned index)
{
    return (PgFont) {
        .v = v,
        .data = data,
        .size = size,
        .index = index,
    };
}


void
pg_free_font(PgFont *font)
{
    if (!font)
        return;

    if (!font->v && !font->v->free)
        return;

    font->v->free(font);
    _pgunmap_file((void*) font->data, font->size);
    free(font);
}


float
pg_font_number(PgFont *font, PgFontProp id)
{
    if (!font)
        return 0.0f;

    if (!font->v && !font->v->number)
        return 0.0f;

    return font->v->number(font, id);
}


int
pg_font_int(PgFont *font, PgFontProp id)
{
    if (!font)
        return 0.0f;

    if (!font->v && !font->v->_int)
        return 0.0f;

    return font->v->_int(font, id);
}


const char*
pg_font_string(PgFont *font, PgFontProp id)
{
    if (!font)
        return "";

    if (!font->v || !font->v->string)
        return "";

    return font->v->string(font, id);
}


PgFont*
pg_scale_font(PgFont *font, float sx, float sy)
{
    if (!font)
        return font;

    if (!font->v || !font->v->scale)
        return font;

    if (sx == 0.0f && sy == 0.0f)
        return font;

    // Fill in sy or sx if the other is set.
    if (sx == 0.0f)
        sx = sy;
    else if (sy == 0.0f)
        sy = sx;

    font->v->scale(font, sx, sy);
    return font;
}


float
pg_font_height(PgFont *font)
{
    return pg_font_number(font, PG_FONT_EM);
}


unsigned
pg_get_glyph(PgFont *font, uint32_t codepoint)
{
    if (!font)
        return 0;

    if (!font->v || !font->v->get_glyph)
        return 0;

    return font->v->get_glyph(font, codepoint);
}


float
pg_glyph_path(Pg *g, PgFont *font, float x, float y, unsigned glyph)
{
    if (!g)
        return 0.0f;

    if (!font)
        return 0.0f;

    if (!font->v && !font->v->glyph_path)
        return 0.0f;

    if (glyph >= (unsigned) pg_font_int(font, PG_FONT_NGLYPHS))
        return 0.0f;

    font->v->glyph_path(g, font, x, y, glyph);

    return x + pg_measure_glyph(font, glyph);
}


float
pg_char_path(Pg *g, PgFont *font, float x, float y, uint32_t codepoint)
{
    return pg_glyph_path(g, font, x, y, pg_get_glyph(font, codepoint));
}


float
pg_chars_path(Pg *g, PgFont *font, float x, float y, const char *str, size_t nbytes)
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
        xi = pg_char_path(g, font, xi, y, pg_read_utf8(&i, end));

    return xi;
}


float
pg_string_path(Pg *g, PgFont *font, float x, float y, const char *str)
{
    if (!str)
        return 0.0f;

    return pg_chars_path(g, font, x, y, str, strlen(str));
}


float
pg_vprintf(Pg *g, PgFont *font, float x, float y, const char *str, va_list ap)
{
    if (!g)
        return 0.0f;

    if (!font)
        return 0.0f;

    if (!str)
        return 0.0f;

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

        float new_x = pg_chars_path(g, font, x, y, buf, line_len);

        max_x = fmaxf(max_x, new_x);
        y += pg_font_height(font);

        if (!buf[line_len])
            break;

        buf += line_len + 1;
    }

    if (allocate)
        free(buf);

    return max_x;
}


float
pg_printf(Pg *g, PgFont *font, float x, float y, const char *str, ...)
{
    va_list ap;

    va_start(ap, str);
    float new_x = pg_vprintf(g, font, x, y, str, ap);
    va_end(ap);

    return new_x;
}


float
pg_measure_glyph(PgFont *font, unsigned glyph)
{
    if (!font)
        return 0.0f;

    if (!font->v || !font->v->measure_glyph)
        return 0.0f;

    if (glyph >= (unsigned) pg_font_number(font, PG_FONT_NGLYPHS))
        return 0.0f;

    return font->v->measure_glyph(font, glyph);
}


float
pg_measure_char(PgFont *font, uint32_t codepoint)
{
    return pg_measure_glyph(font, pg_get_glyph(font, codepoint));
}


float
pg_measure_chars(PgFont *font, const char *str, size_t nbytes)
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
        width += pg_measure_char(font, pg_read_utf8(&i, end));

    return width;
}


float
pg_measure_string(PgFont *font, const char *str)
{
    if (!font)
        return 0.0f;

    if (!str)
        return 0.0f;

    return pg_measure_chars(font, str, strlen(str));
}


unsigned
pg_fit_chars(PgFont *font, const char *s, size_t nbytes, float width)
{
    if (font && s) {
        float       total = 0.0f;
        unsigned    nfit = 0;
        const char  *i = s;
        const char  *end = i + nbytes;

        while (i < end) {
            float size = pg_measure_char(font, pg_read_utf8(&i, end));

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
pg_fit_string(PgFont *font, const char *str, float width)
{
    if (!font)
        return 0;

    if (!str)
        return 0;

    return pg_fit_chars(font, str, strlen(str), width);
}
