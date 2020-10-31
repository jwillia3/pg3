# PLAIN GRAPHICS LIBRARY 3

## Synopsis
This is a vector graphics library that is modeled after the PostScript/PDF
imaging model.

### Table of Contents
- [Canvas](#Canvas)
- [Path](#Path)
  - [Filling and Stroking](#Filling-and-Stroking)
  - [Traversal](#Traversal)
- [Paint](#Paint)
  - [Colour Space](#Colour-Space)
- [State](#State)
- [Text and Fonts](#Text-and-Fonts)



## Canvas
A canvas must be created in order to draw.
A canvas holds information about the surface it draws on.
Notably:
- Size
- Current Path (see [Path](#Path))
- State (see [State](#State))



## Path
In order to draw to a canvas, you must construct a path and then fill it to
color the inside of the path or stroke it to draw lines around the edge of the
path.

Paths are constructed by moving a pen around.
Paths begin with a `pg_move_to()` to set the starting position.
The path is traced using `pg_line_to()`, `pg_curve3_to()`, and
`pg_curve4_to()` to trace lines, quadratic, and cubic Bezier curves,
respectively.

To draw a closed shape, the path should be closed with `pg_close_path()`
which will draw a straight line to position of the last `pg_move_to()`.
If the path is not closed, `pg_fill()` acts the same.
`pg_stroke()` however, will not connect the last point to the first.

If you intend a shape to be closed, you *must* call `pg_close_path()`.
Otherwise, the last point will not be properly connected to the first
*even if* they are at the same coordinates.

Moving the pen before filling or stroking causes a new _subpath_ to be
created. After this, `pg_close_path()` will connect to the last place
the pen was moved. When moving the pen, the previous path is not closed
automatically.

### Filling and Stroking
Once the path has been traced, it can be drawn to the canvas by calling
`pg_fill()` to paint the inside of the path, `pg_stroke()` to draw lines
around the edge of the path, or `pg_fill_stroke()` to do both.
A few parts of the canvas state effect how the path is drawn.
See [State](#State) for details.

### Traversal
Your own code can traverse paths by calling `pg_get_path()`.
With this path object, you can traverse its parts which represent each call
such as `pg_move_to()`, `pg_line_to()`, etc.

```
struct Pgpath {
    unsigned    nparts;
    Pgpart      *parts;
};

typedef enum Pgpart_form {
    PG_PART_MOVE,
    PG_PART_LINE,
    PG_PART_CURVE3,
    PG_PART_CURVE4,
    PG_PART_CLOSE,
} Pgpart_form;

struct Pgpart {
    Pgpart_form form;
    Pgpt        pt[3];
};
```

Example:
```
Pgpath path = pg_get_path(g);
for (unsigned i = 0; i < path.nparts; i++) {
    Pgpt *p = path.parts[i].pt;
    switch (path.parts[i].form) {
    case PG_PART_MOVE:
        printf("pg_move_to(g, %g, %g);\n", p[0].x, p[0].y);
        break;
    case PG_PART_LINE:
        printf("pg_line_to(g, %g, %g);\n", p[0].x, p[0].y);
        break;
    case PG_PART_CURVE3:
        printf("pg_curve3_to(g, %g, %g, %g, %g);\n",
            p[0].x, p[0].y, p[1].x, p[1].y);
        break;
    case PG_PART_CURVE4:
        printf("pg_curve4_to(g, %g, %g, %g, %g, %g, %g);\n",
            p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
        break;
    case PG_PART_CLOSE:
        printf("pg_close_path(g); /* %g, %g */\n",
            p[0].x, p[0].y);
        break;
    }
}
```



## Paint
Paints determine the colour and pattern drawn to the canvas when a
path is stroked or filled.

A simple solid colour paint is created with `pg_solid()`.

A linear gradient pattern is created with `pg_linear()`.
The gradient spans between the two points specified.
The gradient colours are specified by adding stops spanning
from 0 (at the first point) to 1 (at the second point).
The calculations and meaning of linear gradients can be found in the
PostScript Language Reference.

An attempt to add a stop outside of 0 to 1 or a stop before one
that has already been added, it will be ignored.

### Colour Space
The colour space determines the meaning of the four colour components.
`Pgcolor` does not specify the colour space; it only has meaning when
attached to a paint.

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
  - 1931 XYZ colour space
  - XYZ is usually used as a device-independent intermediate colour space
  - `A` transparency



## State
The state of a canvas controls how shapes appear on the cavnas.
Where there is an analogue, the definition of these elements match
the definition according to the PostScript Language Reference
unless otherwise noted.

- CTM
  - Current Transform Matrix
  - Transforms user space coordinates to system coordinates
  - See PostScript Language Reference for a detailed description
  - Default: `{1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}`
- Clip
  - Clips drawing outside given rectangle
  - Resizing canvas resets the clip rectangle
  - Default: `{ {0.0f, 0.0f}, {width, height} }`
- Fill
  - Paint used to fill a path
  - Default: `pg_solidf(0.0f, 0.0f, 0.0f, 1.0f)`
- Stroke
  - Paint used to stroke a path
  - Default: `pg_solidf(0.0f, 0.0f, 0.0f, 1.0f)`
- Line Width
  - Width in user coordinate units of lines when stroking a path
  - Default: `1.0f`
- Line Cap
  - The shape of the end of an unclosed line
  - `PG_BUTT_CAP` ends the line on the given coordinate
  - `PG_SQUARE_CAP` extends the line according to the line width
  - Default: `PG_BUTT_CAP`
- Flatness
  - The flatness tolerance when flattening Bezier curves
  - This represents the number of pixels the flattened curve can be off
  - Lower values are more precise but may require more space and time
  - The valid range is `0.0f - 100.0f`
  - Default: `1.0f`
- Fill Rule
  - The method fill should use to determine what is inside the path
  - `PG_NONZERO_RULE`
  - `PG_EVEN_ODD_RULE`
  - Default: `PG_NONZERO_RULE`
- Text Position
  - The meaning of the coordinate given to text drawing functions
  - `PG_TEXT_POS_TOP` - coordinate is of the ascender
  - `PG_TEXT_POS_BOTTOM` - coordinate is of the descender
  - `PG_TEXT_POS_BASELINE` - coordinate is on the baseline
  - `PG_TEXT_POS_CENTER` - coordinate is in the center
  - Default: `PG_TEXT_POS_TOP`


## Text and Fonts
Text is drawn by finding a font with `pg_find_font()`, scaling it to
a given pixel size with `pg_scale_font()`, tracing its path with
`pg_string_path()`, and filling the path with `pg_fill()`.
Alternatively, `pg_printf()` traces formatted text in the same format
as `printf()`.

`pg_find_font()` attempts to find the font with the given family name
(e.g. 'Helvetica', 'Arial', or 'URW Bookman').
The family is case insensitive but no other normalisation is done
(e.g. space trimming). If the family is not found, null is returned.
If it finds the given font family, the closest match to the given
weight (or 400 - Regular if 0 is given) and slant is returned.
Matching prefers faces that are closest in width class primarily,
then in weight, then in slant.

`pg_measure_string()` returns the size of a string in the given font.
`pg_fit_string()` returns the number of characters that can fit within a given
number of pixels (horizontally).

The fonts of the system can be listed with `pg_list_fonts()`.
An array of `Pgfamily` is returned in alphabetical order by family name.
The array is terminated by a `Pgfamily` with a NULL name.

```
struct Pgfamily {
    const char  *name;
    unsigned    nfaces;
    Pgface      *faces;
};

struct Pgface {
    const char  *family;
    const char  *style;
    const char  *path;
    unsigned    index;
    unsigned    width;
    unsigned    weight;
    bool        is_fixed;
    bool        is_italic;
    bool        is_serif;
    bool        is_sans_serif;
    uint8_t     style_class;
    uint8_t     style_subclass;
    char        panose[10];
};

for (Pgfamily *fam = pg_list_fonts(); fam->name; fam++)
    for (Pgface *fac = fam->faces; fac->family; fac++)
        printf("%s %s\n", fam->name, fac->style);
```

