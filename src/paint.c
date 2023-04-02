#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg-internal-canvas.h>


typedef struct {
    char            name[24];
    PgColorSpace    cspace;
    float           u, v, w, a;
} Spec;

static Spec         specs[] = {
    {"aliceblue",PG_LINEAR_RGB, 0.941176,0.972549,1, 1},
    {"antiquewhite",PG_LINEAR_RGB, 0.980392,0.921569,0.843137, 1},
    {"aqua",PG_LINEAR_RGB, 0,1,1, 1},
    {"aquamarine",PG_LINEAR_RGB, 0.498039,1,0.831373, 1},
    {"azure",PG_LINEAR_RGB, 0.941176,1,1, 1},
    {"beige",PG_LINEAR_RGB, 0.960784,0.960784,0.862745, 1},
    {"bisque",PG_LINEAR_RGB, 1,0.894118,0.768627, 1},
    {"black",PG_LINEAR_RGB, 0,0,0, 1},
    {"blanchedalmond",PG_LINEAR_RGB, 1,0.921569,0.803922, 1},
    {"blue",PG_LINEAR_RGB, 0,0,1, 1},
    {"blueviolet",PG_LINEAR_RGB, 0.541176,0.168627,0.886275, 1},
    {"brown",PG_LINEAR_RGB, 0.647059,0.164706,0.164706, 1},
    {"burlywood",PG_LINEAR_RGB, 0.870588,0.721569,0.529412, 1},
    {"cadetblue",PG_LINEAR_RGB, 0.372549,0.619608,0.627451, 1},
    {"chartreuse",PG_LINEAR_RGB, 0.498039,1,0, 1},
    {"chocolate",PG_LINEAR_RGB, 0.823529,0.411765,0.117647, 1},
    {"clear",PG_LINEAR_RGB, 0,0,0, 0},
    {"coral",PG_LINEAR_RGB, 1,0.498039,0.313725, 1},
    {"cornflowerblue",PG_LINEAR_RGB, 0.392157,0.584314,0.929412, 1},
    {"cornsilk",PG_LINEAR_RGB, 1,0.972549,0.862745, 1},
    {"crimson",PG_LINEAR_RGB, 0.862745,0.0784314,0.235294, 1},
    {"cyan",PG_LINEAR_RGB, 0,1,1, 1},
    {"darkblue",PG_LINEAR_RGB, 0,0,0.545098, 1},
    {"darkcyan",PG_LINEAR_RGB, 0,0.545098,0.545098, 1},
    {"darkgoldenrod",PG_LINEAR_RGB, 0.721569,0.52549,0.0431373, 1},
    {"darkgray",PG_LINEAR_RGB, 0.662745,0.662745,0.662745, 1},
    {"darkgreen",PG_LINEAR_RGB, 0,0.392157,0, 1},
    {"darkgrey",PG_LINEAR_RGB, 0.662745,0.662745,0.662745, 1},
    {"darkkhaki",PG_LINEAR_RGB, 0.741176,0.717647,0.419608, 1},
    {"darkmagenta",PG_LINEAR_RGB, 0.545098,0,0.545098, 1},
    {"darkolivegreen",PG_LINEAR_RGB, 0.333333,0.419608,0.184314, 1},
    {"darkorange",PG_LINEAR_RGB, 1,0.54902,0, 1},
    {"darkorchid",PG_LINEAR_RGB, 0.6,0.196078,0.8, 1},
    {"darkred",PG_LINEAR_RGB, 0.545098,0,0, 1},
    {"darksalmon",PG_LINEAR_RGB, 0.913725,0.588235,0.478431, 1},
    {"darkseagreen",PG_LINEAR_RGB, 0.560784,0.737255,0.560784, 1},
    {"darkslateblue",PG_LINEAR_RGB, 0.282353,0.239216,0.545098, 1},
    {"darkslategray",PG_LINEAR_RGB, 0.184314,0.309804,0.309804, 1},
    {"darkslategrey",PG_LINEAR_RGB, 0.184314,0.309804,0.309804, 1},
    {"darkturquoise",PG_LINEAR_RGB, 0,0.807843,0.819608, 1},
    {"darkviolet",PG_LINEAR_RGB, 0.580392,0,0.827451, 1},
    {"deeppink",PG_LINEAR_RGB, 1,0.0784314,0.576471, 1},
    {"deepskyblue",PG_LINEAR_RGB, 0,0.74902,1, 1},
    {"dimgray",PG_LINEAR_RGB, 0.411765,0.411765,0.411765, 1},
    {"dimgrey",PG_LINEAR_RGB, 0.411765,0.411765,0.411765, 1},
    {"dodgerblue",PG_LINEAR_RGB, 0.117647,0.564706,1, 1},
    {"firebrick",PG_LINEAR_RGB, 0.698039,0.133333,0.133333, 1},
    {"floralwhite",PG_LINEAR_RGB, 1,0.980392,0.941176, 1},
    {"forestgreen",PG_LINEAR_RGB, 0.133333,0.545098,0.133333, 1},
    {"fuchsia",PG_LINEAR_RGB, 1,0,1, 1},
    {"gainsboro",PG_LINEAR_RGB, 0.862745,0.862745,0.862745, 1},
    {"ghostwhite",PG_LINEAR_RGB, 0.972549,0.972549,1, 1},
    {"gold",PG_LINEAR_RGB, 1,0.843137,0, 1},
    {"goldenrod",PG_LINEAR_RGB, 0.854902,0.647059,0.12549, 1},
    {"gray",PG_LINEAR_RGB, 0.501961,0.501961,0.501961, 1},
    {"green",PG_LINEAR_RGB, 0,0.501961,0, 1},
    {"greenyellow",PG_LINEAR_RGB, 0.678431,1,0.184314, 1},
    {"grey",PG_LINEAR_RGB, 0.501961,0.501961,0.501961, 1},
    {"honeydew",PG_LINEAR_RGB, 0.941176,1,0.941176, 1},
    {"hotpink",PG_LINEAR_RGB, 1,0.411765,0.705882, 1},
    {"indianred",PG_LINEAR_RGB, 0.803922,0.360784,0.360784, 1},
    {"indigo",PG_LINEAR_RGB, 0.294118,0,0.509804, 1},
    {"ivory",PG_LINEAR_RGB, 1,1,0.941176, 1},
    {"khaki",PG_LINEAR_RGB, 0.941176,0.901961,0.54902, 1},
    {"lavender",PG_LINEAR_RGB, 0.901961,0.901961,0.980392, 1},
    {"lavenderblush",PG_LINEAR_RGB, 1,0.941176,0.960784, 1},
    {"lawngreen",PG_LINEAR_RGB, 0.486275,0.988235,0, 1},
    {"lemonchiffon",PG_LINEAR_RGB, 1,0.980392,0.803922, 1},
    {"lightblue",PG_LINEAR_RGB, 0.678431,0.847059,0.901961, 1},
    {"lightcoral",PG_LINEAR_RGB, 0.941176,0.501961,0.501961, 1},
    {"lightcyan",PG_LINEAR_RGB, 0.878431,1,1, 1},
    {"lightgoldenrodyellow",PG_LINEAR_RGB, 0.980392,0.980392,0.823529, 1},
    {"lightgray",PG_LINEAR_RGB, 0.827451,0.827451,0.827451, 1},
    {"lightgreen",PG_LINEAR_RGB, 0.564706,0.933333,0.564706, 1},
    {"lightgrey",PG_LINEAR_RGB, 0.827451,0.827451,0.827451, 1},
    {"lightpink",PG_LINEAR_RGB, 1,0.713725,0.756863, 1},
    {"lightsalmon",PG_LINEAR_RGB, 1,0.627451,0.478431, 1},
    {"lightseagreen",PG_LINEAR_RGB, 0.12549,0.698039,0.666667, 1},
    {"lightskyblue",PG_LINEAR_RGB, 0.529412,0.807843,0.980392, 1},
    {"lightslategray",PG_LINEAR_RGB, 0.466667,0.533333,0.6, 1},
    {"lightslategrey",PG_LINEAR_RGB, 0.466667,0.533333,0.6, 1},
    {"lightsteelblue",PG_LINEAR_RGB, 0.690196,0.768627,0.870588, 1},
    {"lightyellow",PG_LINEAR_RGB, 1,1,0.878431, 1},
    {"lime",PG_LINEAR_RGB, 0,1,0, 1},
    {"limegreen",PG_LINEAR_RGB, 0.196078,0.803922,0.196078, 1},
    {"linen",PG_LINEAR_RGB, 0.980392,0.941176,0.901961, 1},
    {"magenta",PG_LINEAR_RGB, 1,0,1, 1},
    {"maroon",PG_LINEAR_RGB, 0.501961,0,0, 1},
    {"mediumaquamarine",PG_LINEAR_RGB, 0.4,0.803922,0.666667, 1},
    {"mediumblue",PG_LINEAR_RGB, 0,0,0.803922, 1},
    {"mediumorchid",PG_LINEAR_RGB, 0.729412,0.333333,0.827451, 1},
    {"mediumpurple",PG_LINEAR_RGB, 0.576471,0.439216,0.858824, 1},
    {"mediumseagreen",PG_LINEAR_RGB, 0.235294,0.701961,0.443137, 1},
    {"mediumslateblue",PG_LINEAR_RGB, 0.482353,0.407843,0.933333, 1},
    {"mediumspringgreen",PG_LINEAR_RGB, 0,0.980392,0.603922, 1},
    {"mediumturquoise",PG_LINEAR_RGB, 0.282353,0.819608,0.8, 1},
    {"mediumvioletred",PG_LINEAR_RGB, 0.780392,0.0823529,0.521569, 1},
    {"midnightblue",PG_LINEAR_RGB, 0.0980392,0.0980392,0.439216, 1},
    {"mintcream",PG_LINEAR_RGB, 0.960784,1,0.980392, 1},
    {"mistyrose",PG_LINEAR_RGB, 1,0.894118,0.882353, 1},
    {"moccasin",PG_LINEAR_RGB, 1,0.894118,0.709804, 1},
    {"navajowhite",PG_LINEAR_RGB, 1,0.870588,0.678431, 1},
    {"navy",PG_LINEAR_RGB, 0,0,0.501961, 1},
    {"oldlace",PG_LINEAR_RGB, 0.992157,0.960784,0.901961, 1},
    {"olive",PG_LINEAR_RGB, 0.501961,0.501961,0, 1},
    {"olivedrab",PG_LINEAR_RGB, 0.419608,0.556863,0.137255, 1},
    {"orange",PG_LINEAR_RGB, 1,0.647059,0, 1},
    {"orangered",PG_LINEAR_RGB, 1,0.270588,0, 1},
    {"orchid",PG_LINEAR_RGB, 0.854902,0.439216,0.839216, 1},
    {"palegoldenrod",PG_LINEAR_RGB, 0.933333,0.909804,0.666667, 1},
    {"palegreen",PG_LINEAR_RGB, 0.596078,0.984314,0.596078, 1},
    {"paleturquoise",PG_LINEAR_RGB, 0.686275,0.933333,0.933333, 1},
    {"palevioletred",PG_LINEAR_RGB, 0.858824,0.439216,0.576471, 1},
    {"papayawhip",PG_LINEAR_RGB, 1,0.937255,0.835294, 1},
    {"peachpuff",PG_LINEAR_RGB, 1,0.854902,0.72549, 1},
    {"peru",PG_LINEAR_RGB, 0.803922,0.521569,0.247059, 1},
    {"pink",PG_LINEAR_RGB, 1,0.752941,0.796078, 1},
    {"plum",PG_LINEAR_RGB, 0.866667,0.627451,0.866667, 1},
    {"powderblue",PG_LINEAR_RGB, 0.690196,0.878431,0.901961, 1},
    {"purple",PG_LINEAR_RGB, 0.501961,0,0.501961, 1},
    {"red",PG_LINEAR_RGB, 1,0,0, 1},
    {"rosybrown",PG_LINEAR_RGB, 0.737255,0.560784,0.560784, 1},
    {"royalblue",PG_LINEAR_RGB, 0.254902,0.411765,0.882353, 1},
    {"saddlebrown",PG_LINEAR_RGB, 0.545098,0.270588,0.0745098, 1},
    {"salmon",PG_LINEAR_RGB, 0.980392,0.501961,0.447059, 1},
    {"sandybrown",PG_LINEAR_RGB, 0.956863,0.643137,0.376471, 1},
    {"seagreen",PG_LINEAR_RGB, 0.180392,0.545098,0.341176, 1},
    {"seashell",PG_LINEAR_RGB, 1,0.960784,0.933333, 1},
    {"sienna",PG_LINEAR_RGB, 0.627451,0.321569,0.176471, 1},
    {"silver",PG_LINEAR_RGB, 0.752941,0.752941,0.752941, 1},
    {"skyblue",PG_LINEAR_RGB, 0.529412,0.807843,0.921569, 1},
    {"slateblue",PG_LINEAR_RGB, 0.415686,0.352941,0.803922, 1},
    {"slategray",PG_LINEAR_RGB, 0.439216,0.501961,0.564706, 1},
    {"slategrey",PG_LINEAR_RGB, 0.439216,0.501961,0.564706, 1},
    {"snow",PG_LINEAR_RGB, 1,0.980392,0.980392, 1},
    {"springgreen",PG_LINEAR_RGB, 0,1,0.498039, 1},
    {"steelblue",PG_LINEAR_RGB, 0.27451,0.509804,0.705882, 1},
    {"tan",PG_LINEAR_RGB, 0.823529,0.705882,0.54902, 1},
    {"teal",PG_LINEAR_RGB, 0,0.501961,0.501961, 1},
    {"thistle",PG_LINEAR_RGB, 0.847059,0.74902,0.847059, 1},
    {"tomato",PG_LINEAR_RGB, 1,0.388235,0.278431, 1},
    {"turquoise",PG_LINEAR_RGB, 0.25098,0.878431,0.815686, 1},
    {"ui-accent", PG_LCHAB, .2, .5, .75, 1},
    {"ui-bg", PG_LCHAB, .9, .0, .0, 1},
    {"ui-dim", PG_LCHAB, .5, .0, .0, 1},
    {"ui-fg", PG_LCHAB, .2, .0, .0, 1},
    {"violet",PG_LINEAR_RGB, 0.933333,0.509804,0.933333, 1},
    {"wheat",PG_LINEAR_RGB, 0.960784,0.870588,0.701961, 1},
    {"white",PG_LINEAR_RGB, 1,1,1, 1},
    {"whitesmoke",PG_LINEAR_RGB, 0.960784,0.960784,0.960784, 1},
    {"yellow",PG_LINEAR_RGB, 1,1,0, 1},
    {"yellowgreen",PG_LINEAR_RGB, 0.603922,0.803922,0.196078, 1},
};

static PgPaint*     paints[sizeof specs / sizeof *specs];

PgPaint*
pg_paint_clone(const PgPaint *from)
{
    if (!from)
        return NULL;
    PgPaint *clone = malloc(sizeof *clone);
    *clone = *from;
    return clone;
}

PgPaint*
pg_paint_new_solid(PgColorSpace cspace, float u, float v, float w, float a)
{
    return pgnew(PgPaint,
                 .type = PG_SOLID_PAINT,
                 .cspace = cspace,
                 .colors[0] = { u, v, w, a },
                 .nstops = 1);
}


PgPaint*
pg_paint_new_linear(PgColorSpace cspace, float ax, float ay, float bx, float by)
{
    return pgnew(PgPaint,
                 .type = PG_LINEAR_PAINT,
                 .cspace = cspace,
                 .colors[0] = (PgColor) {0.0f, 0.0f, 0.0f, 1.0f},
                 .stops[0] = 0.0f,
                 .nstops = 0,
                 .a = pgpt(ax, ay),
                 .b = pgpt(bx, by),
                 .ra = .0f);
}


void
pg_paint_add_stop(PgPaint *paint, float t, float u, float v, float w, float a)
{
    if (!paint)
        return;

    if (paint->nstops + 1 >= 8)
        return;


    // Do not allow inserting out of order.
    float last = paint->nstops
                    ? paint->stops[paint->nstops - 1]
                    : 0.0f;

    if (last > t || t > 1.0f)
        return;

    // Add small distance if t is same as last.
    if (paint->nstops && last == t)
        t = nextafterf(last, last + 1.0f);

    paint->stops[paint->nstops] = t;
    paint->colors[paint->nstops] = (PgColor) { u, v, w, a };
    paint->nstops++;
}


void
pg_paint_clear_stops(PgPaint *paint)
{
    if (!paint)
        return;
    paint->nstops = 0;
}


void
pg_paint_free(PgPaint *paint)
{
    if (!paint)
        return;

    free(paint);
}



static
int
find(const char *name)
{
    int j = sizeof specs / sizeof *specs;
    int i = 0;
    while (i < j) {
        int m = (i + j) / 2;
        int r = strcmp(name, specs[m].name);
        if (r == 0)
            return m;
        if (r < 0)
            j = m;
        else
            i = m + 1;
    }
    return -1;
}


const PgPaint*
pg_paint_from_name(const char *name)
{
    if (!name)
        return 0;

    int index = find(name);
    if (index < 0)
        return 0;

    if (paints[index] == NULL)
        paints[index] = pg_paint_new_solid(specs[index].cspace,
                                           specs[index].u,
                                           specs[index].v,
                                           specs[index].w,
                                           specs[index].a);
    return paints[index];
}


PgColor
pg_color_lch_to_lab(PgColor lch)
{
    float c = lch.v;
    float h = lch.w * 2.0f * 3.14159f;
    return (PgColor) {lch.u, c * cosf(h), c * sinf(h), lch.a};
}


PgColor
pg_color_lab_to_xyz(PgColor lab)
{
    float l = lab.u;
    float y = (l + 16.0f) / 116.0f;
    float x = y + lab.v / 500.0f;
    float z = y - lab.w / 200.0f;
    x = x > 24.0f / 116.0f? x*x*x: (x - 16.0f/116.0f) * 108.0f/841.0f;
    y = y > 24.0f / 116.0f? y*y*y: (y - 16.0f/116.0f) * 108.0f/841.0f;
    z = z > 24.0f / 116.0f? z*z*z: (z - 16.0f/116.0f) * 108.0f/841.0f;
    return (PgColor) {x * 950.4f, y * 1000.0f, z * 1088.8f, lab.a};
}


PgColor
pg_color_xyz_to_rgb(PgColor xyz)
{
    float x = xyz.u;
    float y = xyz.v;
    float z = xyz.w;
    float r = x * +3.2404542f + y * -1.5371385f + z * -0.4985314f;
    float g = x * -0.9692660f + y * +1.8760108f + z * +0.0415560f;
    float b = x * +0.0556434f + y * -0.2040259f + z * +1.0572252f;
    return (PgColor) {r, g, b, xyz.a};
}


PgColor
pg_color_gamma_correct(PgColor rgb, float gamma)
{
    float r = rgb.u;
    float g = rgb.v;
    float b = rgb.w;
    return (PgColor) {
        r < 0.0031308f? 12.92f * r: 1.055f * powf(r, 1.0f / gamma) - 0.055f,
        g < 0.0031308f? 12.92f * g: 1.055f * powf(g, 1.0f / gamma) - 0.055f,
        b < 0.0031308f? 12.92f * b: 1.055f * powf(b, 1.0f / gamma) - 0.055f,
        rgb.a
    };
}


PgColor
pg_color_to_rgb(PgColorSpace cspace, PgColor color, float gamma)
{
    return
        cspace == PG_LINEAR_RGB? pg_color_gamma_correct(color, gamma):
        cspace == PG_LCHAB? pg_color_gamma_correct(pg_color_xyz_to_rgb(pg_color_lab_to_xyz(pg_color_lch_to_lab(color))), gamma):
        cspace == PG_LAB? pg_color_gamma_correct(pg_color_xyz_to_rgb(pg_color_lab_to_xyz(color)), gamma):
        cspace == PG_XYZ? pg_color_gamma_correct(pg_color_xyz_to_rgb(color), gamma):
        color;
}
