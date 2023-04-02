typedef struct PgFontFunc   PgFontFunc;

struct PgFont {
    const PgFontFunc    *v;
    const uint8_t       *data;
    size_t              size;
    unsigned            index;

    float               sx;
    float               sy;
    unsigned            nglyphs;
    float               units;
    float               ascender;
    float               descender;
    char                *prop_buf;
};

struct PgFontFunc {
    unsigned    (*get_glyph)(PgFont *font, uint32_t codepoint);
    void        (*glyph_path)(Pg *g, PgFont *font, float x, float y, uint32_t glyph);
    float       (*measure_glyph)(PgFont *font, uint32_t glyph);
    void        (*scale)(PgFont *font, float sx, float sy);
    float       (*number)(PgFont *font, PgFontProp id);
    int         (*_int)(PgFont *font, PgFontProp id);
    const char  *(*string)(PgFont *font, PgFontProp id);
    PgFont      *(*clone)(PgFont *font);
    void        (*free)(PgFont *font);
};

PgFont _pg_font_init(const PgFontFunc *v,
                    const uint8_t *data,
                    size_t size,
                    unsigned index,
                    unsigned nglyphs,
                    float units,
                    float ascender,
                    float descender);

const char *_pg_fallback_font_substitute(const char *family);
