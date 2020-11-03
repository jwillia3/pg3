Paint
----------------------------------------------------------------

    enum PgPaintType {
        PG_SOLID_PAINT,
        PG_LINEAR_PAINT,
    };

    enum PgColorSpace {
        PG_SRGB,
        PG_LCHAB,
        PG_LAB,
        PG_XYZ,
    };

    struct PgColor {
        float x;
        float y;
        float z;
        float a;
    };

    struct PgPaint {
        PgPaintType     type;
        PgColorSpace    cspace;
        PgColor         colors[8];
        float           stops[8];
        unsigned        nstops;
        PgPt            a;
        PgPt            b;
        float           ra;
        float           rb;
    };

    PgPaint pg_linear(PgColorSpace cspace, float ax, float ay, float bx, float by);
    PgPaint pg_solid(PgColorSpace cspace, float x, float y, float z, float a);
    void pg_add_stop(PgPaint *paint, float t, float x, float y, float z, float a);

Paints determine the color and pattern drawn to the canvas when
a path is stroked or filled.

A simple solid color paint is created with `pg_solid()`.

A linear gradient pattern is created with `pg_linear()`. The
gradient spans between the two points specified. The gradient
colors are specified by adding stops spanning from 0 (at the
first point) to 1 (at the second point). The calculations and
meaning of linear gradients can be found in the PostScript
Language Reference.

An attempt to add a stop outside of 0 to 1 or a stop before one
that has already been added, it will be ignored.



Colour Space
----------------------------------------------------------------

The color space determines the meaning of the four color
components. `PgColor` does not specify the color space; it only
has meaning when attached to a paint.

Colour Spaces

- sRGB
  - Inputs are linear and gamma corrected on output
  - `A` transparency

- CIE LCHab
  - Perceptually uniform
  - `L*` lightness: 0 to 1
  - `C` chroma (relative saturation): 0 to 1
  - `H` hue angle: 0 to 1 (mapping to 0 to 360°)
  - `A` transparency

- CIE LAB
  - Perceptually uniform
  - `L*` lightness: 0 to 1
  - `a*` green-red: -1 to 1
  - `b*` blue-yellow: -1 to 1
  - `A` transparency

- CIE XYZ
  - 1931 XYZ color space
  - XYZ is usually used as a device-independent intermediate
    color space
  - `A` transparency
