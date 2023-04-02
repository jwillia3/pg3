static
inline
PgPt
zero()
{
    return (PgPt) { 0.0f, 0.0f };
}


static
inline
PgPt
add(PgPt a, PgPt b)
{
    return pgpt(a.x + b.x, a.y + b.y);
}


static
inline
PgPt
sub(PgPt a, PgPt b)
{
    return pgpt(a.x - b.x, a.y - b.y);
}


static
inline
PgPt
mul(PgPt a, PgPt b)
{
    return pgpt(a.x * b.x, a.y * b.y);
}


static
inline
PgPt
scale_pt(PgPt p, float s)
{
    return pgpt(p.x * s, p.y * s);
}


static
inline
PgPt
midpoint(PgPt a, PgPt b)
{
    return pgpt((a.x + b.x) * 0.5f, (a.y + b.y) * 0.5f);
}


static
inline
bool
is_flat(PgPt a, PgPt b, PgPt c, PgPt d, float flatness)
{
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


static
inline
unsigned
flatten3(PgPt *out, PgPt a, PgPt b, PgPt c, float flatness, int lim)
{
    if (lim == 0 || is_flat(a, b, c, c, flatness)) {
        *out = c;
        return 1;
    }
    else {
        PgPt ab = midpoint(a, b);
        PgPt bc = midpoint(b, c);
        PgPt abc = midpoint(ab, bc);
        unsigned n = flatten3(out, a, ab, abc, flatness, lim - 1);
        return n + flatten3(out + n, abc, bc, c, flatness, lim - 1);
    }
}


static
inline
unsigned
flatten4(PgPt *out, PgPt a, PgPt b, PgPt c, PgPt d, float flatness, int lim)
{
    if (lim == 0 || is_flat(a, b, c, d, flatness)) {
        *out = d;
        return 1;
    }
    else {
        PgPt ab = midpoint(a, b);
        PgPt bc = midpoint(b, c);
        PgPt cd = midpoint(c, d);
        PgPt abc = midpoint(ab, bc);
        PgPt bcd = midpoint(bc, cd);
        PgPt abcd = midpoint(abc, bcd);
        unsigned n = flatten4(out, a, ab, abc, abcd, flatness, lim - 1);
        return n + flatten4(out + n, abcd, bcd, cd, d, flatness, lim - 1);
    }
}
