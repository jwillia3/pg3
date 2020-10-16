void *_pgmap_file(const char *path, size_t *sizep);
void _pgunmap_file(void *ptr, size_t size);
Pgfont *_pgopen_opentype_font(const uint8_t *data, size_t size, unsigned index);
unsigned _pgget_font_dirs(char *dirs[256]);


/*
    Path Curve Flattening.
*/
static inline bool
is_flat(Pgpt a, Pgpt b, Pgpt c, Pgpt d, float flatness) {
    float ux = 3.0f * b.x - 2.0f * a.x - d.x;
    float uy = 3.0f * b.y - 2.0f * a.y - d.y;
    float vx = 3.0f * c.x - 2.0f * d.x - a.x;
    float vy = 3.0f * c.y - 2.0f * d.y - a.y;
    ux *= ux;
    uy *= uy;
    vx *= vx;
    vy *= vy;
    if (ux < vx) ux = vx;
    if (uy < vy) uy = vy;
    return (ux + uy <= 16.0f * flatness * flatness);
}
static inline unsigned
flatten3(Pgpt *out, Pgpt a, Pgpt b, Pgpt c, float flatness, int lim) {
    if (lim == 0 || is_flat(a, b, c, c, flatness)) {
        *out = c;
        return 1;
    } else {
        Pgpt ab = pgmid(a, b);
        Pgpt bc = pgmid(b, c);
        Pgpt abc = pgmid(ab, bc);
        unsigned n = flatten3(out, a, ab, abc, flatness, lim - 1);
        return n + flatten3(out + n, abc, bc, c, flatness, lim - 1);
    }
}
static inline unsigned
flatten4(Pgpt *out, Pgpt a, Pgpt b, Pgpt c, Pgpt d, float flatness, int lim) {
    if (lim == 0 || is_flat(a, b, c, d, flatness)) {
        *out = d;
        return 1;
    } else {
        Pgpt ab = pgmid(a, b);
        Pgpt bc = pgmid(b, c);
        Pgpt cd = pgmid(c, d);
        Pgpt abc = pgmid(ab, bc);
        Pgpt bcd = pgmid(bc, cd);
        Pgpt abcd = pgmid(abc, bcd);
        unsigned n = flatten4(out, a, ab, abc, abcd, flatness, lim - 1);
        return n + flatten4(out + n, abcd, bcd, cd, d, flatness, lim - 1);
    }
}
