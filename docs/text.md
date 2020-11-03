Text and Fonts
----------------------------------------------------------------

    struct PgFont {
        const PgFontImpl    *v;
        const uint8_t       *data;
        size_t              size;
        unsigned            index;
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


Text is drawn by finding a font with `pg_find_font()`, scaling
it to a given pixel size with `pg_scale_font()`, tracing its
path with `pg_string_path()`, and filling the path with
`pg_fill()`. Alternatively, `pg_printf()` traces formatted text
in the same format as `printf()`.


Finding Fonts
----------------------------------------------------------------

`pg_find_font()` attempts to find the font with the given family
name (e.g. 'Helvetica', 'Arial', or 'URW Bookman'). The family
is case insensitive but no other normalisation is done (e.g.
space trimming). If the family is not found, null is returned.
If it finds the given font family, the closest match to the
given weight (or 400 - Regular if 0 is given) and slant is
returned. Matching prefers faces that are closest in width class
primarily, then in weight, then in slant.

Measuring Characters and Strings
----------------------------------------------------------------

`pg_measure_string()` returns the size of a string in the given
font. `pg_fit_string()` returns the number of characters that
can fit within a given number of pixels (horizontally).


Listing Fonts
----------------------------------------------------------------
The fonts of the system can be listed with `pg_list_fonts()`. An
array of `PgFamily` is returned in alphabetical order by family
name. The array is terminated by a `PgFamily` with a NULL name.

    for (PgFamily *fam = pg_list_fonts(); fam->name; fam++)
        for (PgFace *face = fam->faces; face->family; face++)
            printf("%s %s\n", fam->name, face->style);


Implementing
----------------------------------------------------------------
    struct PgFontImpl {
        unsigned    (*get_glyph)(PgFont *font, uint32_t codepoint);
        void        (*glyph_path)(Pg *g, PgFont *font, float x, float y, uint32_t glyph);
        float       (*measure_glyph)(PgFont *font, uint32_t glyph);
        void        (*scale)(PgFont *font, float sx, float sy);
        float       (*number)(PgFont *font, PgFontProp id);
        const char  *(*string)(PgFont *font, PgFontProp id);
        void        (*free)(PgFont *font);
    };

    Pg pg_init_canvas(const PgCanvasImpl *v, float width, float height);
