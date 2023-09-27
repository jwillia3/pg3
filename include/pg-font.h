typedef struct PgFont       PgFont;
typedef struct PgFace       PgFace;
typedef struct PgFamily     PgFamily;
typedef enum PgFontProp     PgFontProp;

enum PgFontProp {
    PG_FONT_FORMAT,
    PG_FONT_INDEX,
    PG_FONT_NFONTS,
    PG_FONT_FAMILY,
    PG_FONT_STYLE,
    PG_FONT_FULL_NAME,
    PG_FONT_IS_FIXED,
    PG_FONT_IS_ITALIC,
    PG_FONT_IS_SANS_SERIF,
    PG_FONT_IS_SERIF,
    PG_FONT_WEIGHT,
    PG_FONT_WIDTH_CLASS,
    PG_FONT_STYLE_CLASS,
    PG_FONT_STYLE_SUBCLASS,
    PG_FONT_ANGLE,
    PG_FONT_PANOSE_1,
    PG_FONT_PANOSE_2,
    PG_FONT_PANOSE_3,
    PG_FONT_PANOSE_4,
    PG_FONT_PANOSE_5,
    PG_FONT_PANOSE_6,
    PG_FONT_PANOSE_7,
    PG_FONT_PANOSE_8,
    PG_FONT_PANOSE_9,
    PG_FONT_PANOSE_10,
    PG_FONT_NGLYPHS,
    PG_FONT_UNITS,
    PG_FONT_EM,
    PG_FONT_AVG_WIDTH,
    PG_FONT_ASCENDER,
    PG_FONT_DESCENDER,
    PG_FONT_LINEGAP,
    PG_FONT_XHEIGHT,
    PG_FONT_CAPHEIGHT,
    PG_FONT_UNDERLINE,
    PG_FONT_UNDERLINE_SIZE,
    PG_FONT_SUB_X,
    PG_FONT_SUB_Y,
    PG_FONT_SUB_SX,
    PG_FONT_SUB_SY,
    PG_FONT_SUP_X,
    PG_FONT_SUP_Y,
    PG_FONT_SUP_SX,
    PG_FONT_SUP_SY,
};

struct PgFamily {
    const char          *name;
    unsigned            nfaces;
    PgFace              *faces;
};

struct PgFace {
    const char          *family;
    const char          *style;
    const char          *path;
    unsigned            index;
    unsigned            width;
    unsigned            weight;
    bool                is_fixed;
    bool                is_italic;
    bool                is_serif;
    bool                is_sans_serif;
    uint8_t             style_class;
    uint8_t             style_subclass;
    uint8_t             panose[10];
};

PgFont*     pg_font_find(const char *family, unsigned weight, bool italic);
PgFont*     pg_font_clone(PgFont *font);
PgFamily*   pg_font_list(void);
unsigned    pg_font_list_get_count(void);

void        pg_font_free(PgFont *font);

PgFont*     pg_font_scale(PgFont *font, float sx, float sy);

float       pg_font_get_height(PgFont *font);
float       pg_font_measure_char(PgFont *font, uint32_t codepoint);
float       pg_font_measure_chars(PgFont *font, const char *str, size_t nbytes);
float       pg_font_measure_glyph(PgFont *font, uint32_t glyph);
float       pg_font_measure_string(PgFont *font, const char *str);
unsigned    pg_font_fit_chars(PgFont *font, const char *s, size_t nbytes, float width);
unsigned    pg_font_fit_string(PgFont *font, const char *str, float width);

unsigned    pg_font_char_to_glyph(PgFont *font, uint32_t codepoint);

PgPt        pg_font_get_scale(PgFont *font);
PgPt        pg_font_get_em(PgFont *font);
const char* pg_font_prop_string(PgFont *font, PgFontProp id);
float       pg_font_prop_float(PgFont *font, PgFontProp id);
int         pg_font_prop_int(PgFont *font, PgFontProp id);
const char  *pg_font_get_path(PgFont *font);

PgFont*     pg_font_from_data(const char *path, const uint8_t *data, size_t size, unsigned index);
PgFont*     pg_font_from_file(const char *path, unsigned index);
PgFont*     pg_font_from_data_otf(const uint8_t *data, size_t size, unsigned index);
