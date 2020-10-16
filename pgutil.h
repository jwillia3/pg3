#include <stdint.h>
#include <math.h>


/*
    Transform Matrix Manipulation.
*/
static inline Pgpt pgapply_mat(Pgmat ctm, Pgpt p) {
    return pgpt(ctm.a * p.x + ctm.c * p.y + ctm.e,
                ctm.b * p.x + ctm.d * p.y + ctm.f);
}
static inline Pgmat pgmul_mat(Pgmat x, Pgmat y) {
    return (Pgmat) {
        (x.a * y.a) + (x.b * y.c) + (0.0f * y.e),
        (x.a * y.b) + (x.b * y.d) + (0.0f * y.f),
        (x.c * y.a) + (x.d * y.c) + (0.0f * y.e),
        (x.c * y.b) + (x.d * y.d) + (0.0f * y.f),
        (x.e * y.a) + (x.f * y.c) + (1.0f * y.e),
        (x.e * y.b) + (x.f * y.d) + (1.0f * y.f),
    };
}
static inline Pgmat pgident_mat() {
    return (Pgmat) {1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f};
}
static inline Pgmat pgtranslate_mat(Pgmat m, float x, float y) {
    return pgmul_mat(m, (Pgmat) {1.0f, 0.0f, 0.0f, 1.0f, x, y});
}
static inline Pgmat pgscale_mat(Pgmat m, float x, float y) {
    return pgmul_mat(m, (Pgmat) {x, 0.0f, 0.0f, y, 0.0f, 0.0f});
}
static inline Pgmat pgrotate_mat(Pgmat m, float rad) {
    float   sinx = sinf(rad);
    float   cosx = cosf(rad);
    return pgmul_mat(m, (Pgmat) { cosx, sinx, -sinx, cosx, 0, 0 });
}



/*
    Point and Vector Manipulation.
*/
static inline Pgpt pgaddpt(Pgpt a, Pgpt b) { return pgpt(a.x+b.x, a.y+b.y); }
static inline Pgpt pgsubpt(Pgpt a, Pgpt b) { return pgpt(a.x-b.x, a.y-b.y); }
static inline Pgpt pgmulpt(Pgpt a, Pgpt b) { return pgpt(a.x*b.x, a.y*b.y); }
static inline Pgpt pgdivpt(Pgpt a, Pgpt b) { return pgpt(a.x/b.x, a.y/b.y); }
static inline Pgpt pgscalept(Pgpt p, float s) { return pgpt(p.x * s, p.y * s); }
static inline Pgpt pgmid(Pgpt a, Pgpt b) {
    return pgpt((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
}
static inline float pgdist(Pgpt a, Pgpt b) {
    float   dx = a.x - b.x;
    float   dy = a.y - b.y;
    return sqrtf(dx * dx + dy * dy);
}
static inline Pgpt pgnorm(Pgpt p) {
    float invmag = 1.0f / sqrtf(p.x * p.x + p.y * p.y);
    return pgpt(p.x * invmag, p.y * invmag);
}
static inline float pgdot(Pgpt a, Pgpt b) { return a.x * b.x + a.y * b.y; }
static inline Pgpt pg90ccw(Pgpt p) { return pgpt(-p.y, p.x); }
static inline Pgpt pg90cw(Pgpt p) { return pgpt(p.y, -p.x); }



/*
    UTF-8.
*/
static inline unsigned
pgread_utf8_tail(const uint8_t **in, unsigned tail, unsigned min) {
    unsigned c = *(*in)++;
    unsigned got;
    for (got = 0; (**in & 0xc0) == 0x80; got++)
        c = (c << 6) + (*(*in)++ & 0x3f);
    return got == tail && c >= min? c: 0xfffd;
}
static inline uint32_t pgread_utf8(const uint8_t **in) {
    if (**in < 0x80)
        return *(*in)++;
    if (**in < 0xd0)
        return pgread_utf8_tail(in, 1, 0x80) & 0x07ff;
    if (**in < 0xf0)
        return pgread_utf8_tail(in, 2, 0x800) & 0xffff;
    return pgread_utf8_tail(in, 3, 0x10000) & 0x10ffff;
}
static inline uint8_t *pgwrite_utf8(uint8_t *out, uint32_t c) {
    if (c < 0x80)
        *out++ = (uint8_t) c;
    else if (c < 0x0800) {
        *out++ = (uint8_t) (0xc0 + (c >> 6));
        *out++ = (uint8_t) (0x80 + (c & 0x3f));
    } else if (c < 0x10000) {
        *out++ = (uint8_t) (0xe0 + (c >> 12));
        *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
        *out++ = (uint8_t) (0x80 + (c & 0x3f));
    } else if (c < 0x10ffff) {
        *out++ = (uint8_t) (0xe0 + (c >> 18));
        *out++ = (uint8_t) (0x80 + (c >> 12 & 0x3f));
        *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
        *out++ = (uint8_t) (0x80 + (c & 0x3f));
    }
    return out;
}


static inline Pgcolor pglch_to_lab(Pgcolor lch) {
    float c = lch.y;
    float h = lch.z * 2.0f * 3.14159f;
    return (Pgcolor) {lch.x, c * cosf(h), c * sinf(h), lch.a};
}
static inline Pgcolor pglab_to_xyz(Pgcolor lab) {
    float l = lab.x;
    float y = (l + 16.0f) / 116.0f;
    float x = y + lab.y / 500.0f;
    float z = y - lab.z / 200.0f;
    x = x > 24.0f / 116.0f? x*x*x: (x - 16.0f/116.0f) * 108.0f/841.0f;
    y = y > 24.0f / 116.0f? y*y*y: (y - 16.0f/116.0f) * 108.0f/841.0f;
    z = z > 24.0f / 116.0f? z*z*z: (z - 16.0f/116.0f) * 108.0f/841.0f;
    return (Pgcolor) {x * 950.4f, y * 1000.0f, z * 1088.8f, lab.a};
}
static inline Pgcolor pgxyz_to_rgb(Pgcolor xyz) {
    float x = xyz.x;
    float y = xyz.y;
    float z = xyz.z;
    float r = x * +3.2404542f + y * -1.5371385f + z * -0.4985314f;
    float g = x * -0.9692660f + y * +1.8760108f + z * +0.0415560f;
    float b = x * +0.0556434f + y * -0.2040259f + z * +1.0572252f;
    return (Pgcolor) {r, g, b, xyz.a};
}
static inline Pgcolor pggamma(Pgcolor rgb) {
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;
    return (Pgcolor) {
        r < 0.0031308f? 12.92f * r: 1.055f * powf(r, 1.0f / 2.20f) - 0.055f,
        g < 0.0031308f? 12.92f * g: 1.055f * powf(g, 1.0f / 2.20f) - 0.055f,
        b < 0.0031308f? 12.92f * b: 1.055f * powf(b, 1.0f / 2.20f) - 0.055f,
        rgb.a
    };
}
static inline Pgcolor pgconvert_color(Pgcolorspace cspace, Pgcolor colour) {
    return  cspace == 0? pggamma(colour):
            cspace == 1? pggamma(pgxyz_to_rgb(pglab_to_xyz(pglch_to_lab(colour)))):
            cspace == 2? pggamma(pgxyz_to_rgb(pglab_to_xyz(colour))):
            cspace == 3? pggamma(pgxyz_to_rgb(colour)):
            colour;
}
