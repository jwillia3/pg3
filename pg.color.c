#include <math.h>
#include <string.h>
#include <pg3.h>


PgColor
pg_oklch_to_oklab(PgColor oklch)
{
    float c = oklch.y;
    float h = oklch.z * 2.0f * 3.14159f;
    return (PgColor) {oklch.x, c * cosf(h), c * sinf(h), oklch.a};
}


PgColor
pg_oklab_to_rgb(PgColor oklab)
{
    PgColor c = oklab;
    float l_ = c.x + 0.3963377774f * c.y + 0.2158037573f * c.z;
    float m_ = c.x - 0.1055613458f * c.y - 0.0638541728f * c.z;
    float s_ = c.x - 0.0894841775f * c.y - 1.2914855480f * c.z;

    float l = l_ * l_ * l_;
    float m = m_ * m_ * m_;
    float s = s_ * s_ * s_;

    return (PgColor) {
        +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s,
        -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s,
        -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s,
        oklab.a,
    };
}

PgColor
pg_lch_to_lab(PgColor lch)
{
    float c = lch.y;
    float h = lch.z * 2.0f * 3.14159f;
    return (PgColor) {lch.x, c * cosf(h), c * sinf(h), lch.a};
}


PgColor
pg_lab_to_xyz(PgColor lab)
{
    float l = lab.x;
    float y = (l + 16.0f) / 116.0f;
    float x = y + lab.y / 500.0f;
    float z = y - lab.z / 200.0f;
    x = x > 24.0f / 116.0f? x*x*x: (x - 16.0f/116.0f) * 108.0f/841.0f;
    y = y > 24.0f / 116.0f? y*y*y: (y - 16.0f/116.0f) * 108.0f/841.0f;
    z = z > 24.0f / 116.0f? z*z*z: (z - 16.0f/116.0f) * 108.0f/841.0f;
    return (PgColor) {x * 950.4f, y * 1000.0f, z * 1088.8f, lab.a};
}


PgColor
pg_xyz_to_rgb(PgColor xyz)
{
    float x = xyz.x;
    float y = xyz.y;
    float z = xyz.z;
    float r = x * +3.2404542f + y * -1.5371385f + z * -0.4985314f;
    float g = x * -0.9692660f + y * +1.8760108f + z * +0.0415560f;
    float b = x * +0.0556434f + y * -0.2040259f + z * +1.0572252f;
    return (PgColor) {r, g, b, xyz.a};
}


PgColor
pg_gamma_correct(PgColor rgb, float gamma)
{
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;
    return (PgColor) {
        r < 0.0031308f? 12.92f * r: 1.055f * powf(r, 1.0f / gamma) - 0.055f,
        g < 0.0031308f? 12.92f * g: 1.055f * powf(g, 1.0f / gamma) - 0.055f,
        b < 0.0031308f? 12.92f * b: 1.055f * powf(b, 1.0f / gamma) - 0.055f,
        rgb.a
    };
}


PgColor
pg_convert_color_to_rgb(PgColorSpace cspace, PgColor color, float gamma)
{
    return
        cspace == PG_LINEAR_RGB? pg_gamma_correct(color, gamma):
        cspace == PG_OKLCH? pg_gamma_correct(pg_oklab_to_rgb(pg_oklch_to_oklab(color)), gamma):
        cspace == PG_OKLAB? pg_gamma_correct(pg_oklab_to_rgb(color), gamma):
        cspace == PG_LCHAB? pg_gamma_correct(pg_xyz_to_rgb(pg_lab_to_xyz(pg_lch_to_lab(color))), gamma):
        cspace == PG_LAB? pg_gamma_correct(pg_xyz_to_rgb(pg_lab_to_xyz(color)), gamma):
        cspace == PG_XYZ? pg_gamma_correct(pg_xyz_to_rgb(color), gamma):
        color;
}


static
volatile
bool
web_color_paints_inited;

static
struct web_color {
    char *name;
    float r, g, b;
} web_colors[] = {
    {"aliceblue",0.941176,0.972549,1},
    {"antiquewhite",0.980392,0.921569,0.843137},
    {"aqua",0,1,1},
    {"aquamarine",0.498039,1,0.831373},
    {"azure",0.941176,1,1},
    {"beige",0.960784,0.960784,0.862745},
    {"bisque",1,0.894118,0.768627},
    {"black",0,0,0},
    {"blanchedalmond",1,0.921569,0.803922},
    {"blue",0,0,1},
    {"blueviolet",0.541176,0.168627,0.886275},
    {"brown",0.647059,0.164706,0.164706},
    {"burlywood",0.870588,0.721569,0.529412},
    {"cadetblue",0.372549,0.619608,0.627451},
    {"chartreuse",0.498039,1,0},
    {"chocolate",0.823529,0.411765,0.117647},
    {"coral",1,0.498039,0.313725},
    {"cornflowerblue",0.392157,0.584314,0.929412},
    {"cornsilk",1,0.972549,0.862745},
    {"crimson",0.862745,0.0784314,0.235294},
    {"cyan",0,1,1},
    {"darkblue",0,0,0.545098},
    {"darkcyan",0,0.545098,0.545098},
    {"darkgoldenrod",0.721569,0.52549,0.0431373},
    {"darkgray",0.662745,0.662745,0.662745},
    {"darkgreen",0,0.392157,0},
    {"darkgrey",0.662745,0.662745,0.662745},
    {"darkkhaki",0.741176,0.717647,0.419608},
    {"darkmagenta",0.545098,0,0.545098},
    {"darkolivegreen",0.333333,0.419608,0.184314},
    {"darkorange",1,0.54902,0},
    {"darkorchid",0.6,0.196078,0.8},
    {"darkred",0.545098,0,0},
    {"darksalmon",0.913725,0.588235,0.478431},
    {"darkseagreen",0.560784,0.737255,0.560784},
    {"darkslateblue",0.282353,0.239216,0.545098},
    {"darkslategray",0.184314,0.309804,0.309804},
    {"darkslategrey",0.184314,0.309804,0.309804},
    {"darkturquoise",0,0.807843,0.819608},
    {"darkviolet",0.580392,0,0.827451},
    {"deeppink",1,0.0784314,0.576471},
    {"deepskyblue",0,0.74902,1},
    {"dimgray",0.411765,0.411765,0.411765},
    {"dimgrey",0.411765,0.411765,0.411765},
    {"dodgerblue",0.117647,0.564706,1},
    {"firebrick",0.698039,0.133333,0.133333},
    {"floralwhite",1,0.980392,0.941176},
    {"forestgreen",0.133333,0.545098,0.133333},
    {"fuchsia",1,0,1},
    {"gainsboro",0.862745,0.862745,0.862745},
    {"ghostwhite",0.972549,0.972549,1},
    {"gold",1,0.843137,0},
    {"goldenrod",0.854902,0.647059,0.12549},
    {"gray",0.501961,0.501961,0.501961},
    {"green",0,0.501961,0},
    {"greenyellow",0.678431,1,0.184314},
    {"grey",0.501961,0.501961,0.501961},
    {"honeydew",0.941176,1,0.941176},
    {"hotpink",1,0.411765,0.705882},
    {"indianred",0.803922,0.360784,0.360784},
    {"indigo",0.294118,0,0.509804},
    {"ivory",1,1,0.941176},
    {"khaki",0.941176,0.901961,0.54902},
    {"lavender",0.901961,0.901961,0.980392},
    {"lavenderblush",1,0.941176,0.960784},
    {"lawngreen",0.486275,0.988235,0},
    {"lemonchiffon",1,0.980392,0.803922},
    {"lightblue",0.678431,0.847059,0.901961},
    {"lightcoral",0.941176,0.501961,0.501961},
    {"lightcyan",0.878431,1,1},
    {"lightgoldenrodyellow",0.980392,0.980392,0.823529},
    {"lightgray",0.827451,0.827451,0.827451},
    {"lightgreen",0.564706,0.933333,0.564706},
    {"lightgrey",0.827451,0.827451,0.827451},
    {"lightpink",1,0.713725,0.756863},
    {"lightsalmon",1,0.627451,0.478431},
    {"lightseagreen",0.12549,0.698039,0.666667},
    {"lightskyblue",0.529412,0.807843,0.980392},
    {"lightslategray",0.466667,0.533333,0.6},
    {"lightslategrey",0.466667,0.533333,0.6},
    {"lightsteelblue",0.690196,0.768627,0.870588},
    {"lightyellow",1,1,0.878431},
    {"lime",0,1,0},
    {"limegreen",0.196078,0.803922,0.196078},
    {"linen",0.980392,0.941176,0.901961},
    {"magenta",1,0,1},
    {"maroon",0.501961,0,0},
    {"mediumaquamarine",0.4,0.803922,0.666667},
    {"mediumblue",0,0,0.803922},
    {"mediumorchid",0.729412,0.333333,0.827451},
    {"mediumpurple",0.576471,0.439216,0.858824},
    {"mediumseagreen",0.235294,0.701961,0.443137},
    {"mediumslateblue",0.482353,0.407843,0.933333},
    {"mediumspringgreen",0,0.980392,0.603922},
    {"mediumturquoise",0.282353,0.819608,0.8},
    {"mediumvioletred",0.780392,0.0823529,0.521569},
    {"midnightblue",0.0980392,0.0980392,0.439216},
    {"mintcream",0.960784,1,0.980392},
    {"mistyrose",1,0.894118,0.882353},
    {"moccasin",1,0.894118,0.709804},
    {"navajowhite",1,0.870588,0.678431},
    {"navy",0,0,0.501961},
    {"oldlace",0.992157,0.960784,0.901961},
    {"olive",0.501961,0.501961,0},
    {"olivedrab",0.419608,0.556863,0.137255},
    {"orange",1,0.647059,0},
    {"orangered",1,0.270588,0},
    {"orchid",0.854902,0.439216,0.839216},
    {"palegoldenrod",0.933333,0.909804,0.666667},
    {"palegreen",0.596078,0.984314,0.596078},
    {"paleturquoise",0.686275,0.933333,0.933333},
    {"palevioletred",0.858824,0.439216,0.576471},
    {"papayawhip",1,0.937255,0.835294},
    {"peachpuff",1,0.854902,0.72549},
    {"peru",0.803922,0.521569,0.247059},
    {"pink",1,0.752941,0.796078},
    {"plum",0.866667,0.627451,0.866667},
    {"powderblue",0.690196,0.878431,0.901961},
    {"purple",0.501961,0,0.501961},
    {"red",1,0,0},
    {"rosybrown",0.737255,0.560784,0.560784},
    {"royalblue",0.254902,0.411765,0.882353},
    {"saddlebrown",0.545098,0.270588,0.0745098},
    {"salmon",0.980392,0.501961,0.447059},
    {"sandybrown",0.956863,0.643137,0.376471},
    {"seagreen",0.180392,0.545098,0.341176},
    {"seashell",1,0.960784,0.933333},
    {"sienna",0.627451,0.321569,0.176471},
    {"silver",0.752941,0.752941,0.752941},
    {"skyblue",0.529412,0.807843,0.921569},
    {"slateblue",0.415686,0.352941,0.803922},
    {"slategray",0.439216,0.501961,0.564706},
    {"slategrey",0.439216,0.501961,0.564706},
    {"snow",1,0.980392,0.980392},
    {"springgreen",0,1,0.498039},
    {"steelblue",0.27451,0.509804,0.705882},
    {"tan",0.823529,0.705882,0.54902},
    {"teal",0,0.501961,0.501961},
    {"thistle",0.847059,0.74902,0.847059},
    {"tomato",1,0.388235,0.278431},
    {"turquoise",0.25098,0.878431,0.815686},
    {"violet",0.933333,0.509804,0.933333},
    {"wheat",0.960784,0.870588,0.701961},
    {"white",1,1,1},
    {"whitesmoke",0.960784,0.960784,0.960784},
    {"yellow",1,1,0},
    {"yellowgreen",0.603922,0.803922,0.196078},
};

static
PgPaint
web_color_paints[sizeof web_colors / sizeof *web_colors];


static
int
find_web_color(const char *name)
{
    int j = sizeof web_colors / sizeof *web_colors;
    int i = 0;
    while (i < j) {
        int m = (i + j) / 2;
        int r = strcmp(name, web_colors[m].name);
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
pg_web_color_paint(const char *name)
{
    if (!name)
        return 0;

    if (!web_color_paints_inited) {
        for (size_t i = 0; i < sizeof web_colors / sizeof *web_colors; i++)
            web_color_paints[i] = pg_solid(PG_LINEAR_RGB,
                                            web_colors[i].r,
                                            web_colors[i].g,
                                            web_colors[i].b,
                                            1.0f);
    }


    int index = find_web_color(name);
    if (index < 0)
        return 0;
    return web_color_paints + index;
}


PgColor
pg_web_color(const char *name)
{
    int index = find_web_color(name);
    if (index < 0)
        return (PgColor) { 0.0f, 0.0f, 0.0f, 0.0f };
    struct web_color c = web_colors[index];
    return (PgColor) { c.r, c.g, c.b, 1.0f };
}
