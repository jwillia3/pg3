#include <math.h>
#include <pg3.h>

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
pg_gamma_correct(PgColor rgb)
{
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;
    return (PgColor) {
        r < 0.0031308f? 12.92f * r: 1.055f * powf(r, 1.0f / 2.20f) - 0.055f,
        g < 0.0031308f? 12.92f * g: 1.055f * powf(g, 1.0f / 2.20f) - 0.055f,
        b < 0.0031308f? 12.92f * b: 1.055f * powf(b, 1.0f / 2.20f) - 0.055f,
        rgb.a
    };
}


PgColor
pg_convert_color_to_srgb(PgColorSpace cspace, PgColor color)
{
    return
        cspace == PG_SRGB? pg_gamma_correct(color):
        cspace == PG_LCHAB? pg_gamma_correct(pg_xyz_to_rgb(pg_lab_to_xyz(pg_lch_to_lab(color)))):
        cspace == PG_LAB? pg_gamma_correct(pg_xyz_to_rgb(pg_lab_to_xyz(color))):
        cspace == PG_XYZ? pg_gamma_correct(pg_xyz_to_rgb(color)):
        color;
}
