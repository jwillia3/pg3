State
----------------------------------------------------------------

    enum PgLineCap {
        PG_BUTT_CAP,
        PG_SQUARE_CAP,
    };

    enum PgFillRule {
        PG_NONZERO_RULE,
        PG_EVEN_ODD_RULE,
    };

    enum PgTextPos {
        PG_TEXT_POS_TOP,
        PG_TEXT_POS_BOTTOM,
        PG_TEXT_POS_BASELINE,
        PG_TEXT_POS_CENTER,
    };

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

    struct PgState {
      PgTM                ctm;
      PgPaint             fill;
      PgPaint             stroke;
      PgPaint             clear;
      float               line_width;
      PgLineCap           line_cap;
      float               flatness;
      PgFillRule          fill_rule;
      PgRect              clip;
      PgTextPos           text_pos;
      bool                underline;
  };


The state of a canvas controls how shapes appear on the cavnas.
Where there is an analogue, the definition of these elements
match the definition according to the PostScript Language
Reference unless otherwise noted.

- Clear
  - Paint used to clear the canvas
  - Default: `pg_solid(PG_LCHAB, 1.0f, 0.0f, 0.0f, 1.0f)`

- Clip
  - Clips drawing outside given rectangle
  - Resizing canvas resets the clip rectangle
  - Default: `{ {0.0f, 0.0f}, {width, height} }`

- CTM
  - Current Transform Matrix
  - Transforms user space coordinates to system coordinates
  - See PostScript Language Reference for a detailed description
  - Default: `{1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}`

- Fill
  - Paint used to fill a path
  - Default: `pg_solid(PG_LCHAB, 0.0f, 0.0f, 0.0f, 1.0f)`

- Fill Rule
  - The method to determine the inside of paths
  - See the PostScript Language Reference for more information
  - `PG_NONZERO_RULE`
  - `PG_EVEN_ODD_RULE`
  - Default: `PG_NONZERO_RULE`

- Flatness
  - The flatness tolerance when flattening Bezier curves
  - This represents the number of pixels the flattened curve can
    be off
  - Lower values are more precise but may require more space and
    time
  - See the PostScript Language Reference for more information
  - The valid range is `0.0f - 100.0f`
  - Default: `1.0f`

- Line Width
  - Width in user coordinate units of lines when stroking a path
  - Default: `1.0f`

- Line Cap
  - The shape of the end of an unclosed line
  - `PG_BUTT_CAP` ends the line on the given coordinate
  - `PG_SQUARE_CAP` extends the line according to the line width
  - Default: `PG_BUTT_CAP`

- Stroke
  - Paint used to stroke a path
  - Default: `pg_solid(PG_LCHAB, 0.0f, 0.0f, 0.0f, 1.0f)`

- Text Position
  - The meaning of the coordinate given to text drawing
    functions
  - `PG_TEXT_POS_TOP` - coordinate is of the ascender
  - `PG_TEXT_POS_BOTTOM` - coordinate is of the descender
  - `PG_TEXT_POS_BASELINE` - coordinate is on the baseline
  - `PG_TEXT_POS_CENTER` - coordinate is in the center
  - Default: `PG_TEXT_POS_TOP`
