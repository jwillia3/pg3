/*
    Notes:
    - See OpenTypeGuide.txt for the meaning of data and offsets
    - Don't read data directly from file buffer, use peek functions
    - Check bounds before reading data or following a pointer
    - Do NOT cast float to unsigned!
      - ARM: `(unsigned) -1.0f == 0`
      - x86: `(unsigned) -1.0f == (unsigned) -1`
    - Define DEBUG_FONT to get debug info on the console
    - Define TRACE_FONT to get step-by-step trace info on the console
*/

// #define DEBUG_FONT 1
// #define TRACE_FONT 1

#if TRACE_FONT
    #define DBGTRACE(...)   printf(__VA_ARGS__)
#else
    #define DBGTRACE(...)   ((void) 0)
#endif

#ifdef DEBUG_FONT
    #define DBGMSG(ERR)     puts("[DBGTRACE] FAIL: " ERR)
    #define FAIL(ERR)       do { DBGMSG(ERR); goto fail; } while (0)
#else
    #define DBGMSG(ERR)     ((void) 0)
    #define FAIL(ERR)       goto fail
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pg.h>
#include <pg.internal.h>

#define OTF(FONT) ((otf*) pg_get_font_impl((FONT)))

// Four-Character Tags
#define C4(a,b,c,d) ((a << 24) + (b << 16) + (c << 8) + d)

// Bounds Checking (SZ bytes are readable from offset N)
#define BOUNDS(S, I, SZ, MSG)\
    if ((uint64_t) (I) + (uint64_t) (SZ) > (uint64_t) (S).size)\
        FAIL(MSG)
#define BC(I, SZ, MSG)      BOUNDS(cursect, I, SZ, MSG)

// Peek byte, word, and doubleword from current section
#define PB(I)       *(cursect.ptr + (I))
#define PW(I)       peek16(cursect.ptr + (I))
#define PD(I)       peek32(cursect.ptr + (I))
#define PN(I, N)    peekn(cursect.ptr + (I), (N))

// Read word or doubleword from a table in the font.
#define TW(S, I)    peek16((OTF(font)->S).ptr + (I))
#define TD(S, I)    peek32((OTF(font)->S).ptr + (I))

typedef union {
    float       f;
    int32_t     i;
} cffoperand;

typedef struct cffindex {
    unsigned        n;
    unsigned        sz;
    const uint8_t   *offsets;
    const uint8_t   *heap;
} cffindex;

typedef struct {
    const uint8_t   *ptr;
    size_t size;
} section;

typedef struct {
    unsigned        cffver;
    bool            longloca;
    unsigned        nhmtx;

    cffindex        charstrings;
    cffindex        lsubrs;
    cffindex        gsubrs;

    section         glyf;
    section         hmtx;
    section         loca;
    section         name;
    section         os2;

    unsigned        fontindex;
    unsigned        nfonts;
    bool            fixed;
    bool            italic;
    float           angle;

    char            prop_buf[256];
} otf;

static const char *weights[10] = {
    "(not set)",
    "Thin",
    "Extra Light",
    "Light",
    "Normal",
    "Medium",
    "Semibold",
    "Bold",
    "Extra Bold",
    "Black"
};

static const char *widths[10] = {
    "(not set)",
    "Ultra Condensed",
    "Extra Condensed",
    "Condensed",
    "Semi-Condensed",
    "Normal",
    "Semi-Expanded",
    "Expanded",
    "Extra Expanded",
    "Ultra Expanded",
};

static inline uint32_t peekn(const uint8_t *ptr, unsigned n) {
    uint32_t x = 0;
    for (unsigned i = 0; i < n; i++)
        x = (x << 8) + ptr[i];
    return x;
}
static inline uint16_t peek16(const uint8_t *ptr) {
    return (uint16_t) (ptr[0] << 8 | ptr[1]);
}
static inline uint32_t peek32(const uint8_t *ptr) {
    return (uint32_t) peek16(ptr) << 16 | peek16(ptr + 2);
}

static unsigned biassubr(unsigned i, unsigned n) {
    return  n < 1240?   i + 107:
            n < 33900?  i + 1131:
                        i + 32768;
}

// This is used for CFF integers AND Type 2 integers.
// They differ in 29 and 255 so the byte sent to this must be checked first.
static int32_t cffint(section cursect, size_t *offset) {
    const uint8_t   *s = cursect.ptr + *offset;
    uint8_t         x = *s;
    size_t          size =  32  <= x && x <= 246?   1:
                            247 <= x && x <= 250?   2:
                            251 <= x && x <= 254?   2:
                            x == 28?                3:
                            5;
    size_t          limit = cursect.size;
    if (*offset + size > limit) {
        *offset = limit;
        return 0;
    }
    *offset += size;
    return 32 <= x && x <= 246?     (int16_t) (x - 139):
           247 <= x && x <= 250?    (int16_t) ((x - 247) * 256 + s[1] + 108):
           251 <= x && x <= 254?    (int16_t) (-(x - 251) * 256 - s[1] - 108):
           x == 28?                 (int16_t) (s[1] * 256 + s[2]):
           (int32_t) (s[1] << 24 | s[2] << 16 | s[3] << 8 | s[4]);
}

static float cffreal(section cursect, size_t *offset) {
    static const char *map = "0123456789.E__-";
    char            buf[16 + 4];
    size_t          i = *offset;
    unsigned        n = 0;

    while (i < cursect.size) {
        uint8_t x = cursect.ptr[i++];

        if ((x & 0xf0) == 0xc0) buf[n++] = 'E', buf[n++] = '-';
        else                    buf[n++] = map[x >> 4 & 0xf];

        if ((x & 0x0f) == 0x0c) buf[n++] = 'E', buf[n++] = '-';
        else                    buf[n++] = map[x & 0xf];

        if (n > 16) {   // Truncate if we've overflown the buffer.
            buf[16] = 0;
            while (i < cursect.size &&
                  (cursect.ptr[i] & 0xf0) != 0xf0 &&
                  (cursect.ptr[i] & 0xf) != 0xf)
            {
                i++;
            }
            break;
        }

        if ((x & 0xf0) == 0xf0 || (x & 0x0f) == 0x0f)
            break;
    }
    buf[n] = 0;
    *offset = i;
    return strtof(buf, 0);
}

// Get next entry in the dictionary.
// Arguments are put on the passed-in stack.
// -1 is returned on error and processing should halt.
static int
nextdict(section cursect, size_t *p, cffoperand *stack, unsigned *ss) {
    *ss = 0;
    while (*p < cursect.size) {
        unsigned o = PB(*p);

        if (o == 30) {                      // Real operand.
            if (*ss >= 48)
                FAIL("CFF_DICT_STACK_OVERFLOW_REAL");
            (*p)++;
            stack[(*ss)++].f = cffreal(cursect, p);
        }
        else if (o >= 28 && o != 255) {   // Integer operand.
            if (*ss >= 48)
                FAIL("CFF_DICT_STACK_OVERFLOW_INT");
            stack[(*ss)++].i = cffint(cursect, p);
        }
        else if (o == 12) {               // Two-byte operator.
            BC(*p, 2, "TWO_BYTE_OP");
            o = 0x0c00 + PB(*p + 1);
            *p += 2;
            return (int) o;
        }
        else {
            (*p)++;
            return (int) o;
        }
    }
    FAIL("NO_OPERATOR");
fail:
    return -1; // No operator to consume operands.
}

// Read a CFF INDEX and skip past it.
static bool makeindex(section cursect, size_t *p, cffindex *index) {
    BC(*p, 2, "NO_INDEX_SIZE");
    if (PW(*p) == 0) { // Special case zero-element index.
        *p += 2;
        *index = (cffindex) {0, 0, 0, 0};
        return true;
    }
    BC(*p + 2, 1, "NO_INDEX_HEADER");
    unsigned    n = PW(*p);
    unsigned    sz = PB(*p + 2);
    size_t      offsets = *p + 3;
    size_t      heap = offsets + (n + 1) * sz; // Adjusted for offsets +1.

    *index = (cffindex) {n, sz, cursect.ptr + offsets, cursect.ptr + heap};
    if (sz > 4) FAIL("IDX_OFF_SIZE");
    BC(offsets + n * sz, sz, "IDX_OFFS_SHORT");
    *p = heap - 1 + PN(offsets + n * sz, sz);
    BC(*p, 0, "IDX_DATA_SHORT");
    return true;
fail:
    return false;
}

static bool indexindex(cffindex index, unsigned i, section *sect) {
    if (i >= index.n)
        return false;
    unsigned m = peekn(index.offsets + i * index.sz, index.sz);
    unsigned n = peekn(index.offsets + (i + 1) * index.sz, index.sz);
    *sect = (section) {index.heap + m - 1, n - m};
    return true;
}

static bool loadcmap4(uint16_t charmap[65536], section cursect) {
    BC(12, 2, "CMAP_FORMAT4_HEADER");
    if (PW(0) != 4) FAIL("CMAP_SUBTBL_FORMAT");

    cursect.size = PW(2);
    BC(14, PW(6) * 4, "CMAP_FORMAT4_NSEGS");

    unsigned    nsegs = PW(6) / 2;
    unsigned    ends = 14;
    unsigned    starts = ends + nsegs * 2 + 2;
    unsigned    deltas = starts + nsegs * 2;
    unsigned    offsets = deltas + nsegs * 2;

    for (unsigned i = 0; i < nsegs; i++) {
        unsigned    s = PW(starts + i * 2);
        unsigned    e = PW(ends + i * 2);
        signed      d = (int16_t) PW(deltas + i * 2);
        unsigned    o = PW(offsets + i * 2);

        if (o) {
            unsigned p = (offsets + i * 2 + o) & 65535;
            BC(p, (e - s) * 2, "CMAP_FORMAT4_OFFSET");
            for (unsigned c = s; c <= e; c++, p += 2)
                charmap[c] = (uint16_t) (PW(p)? PW(p) + d: 0);
        }
        else
            for (unsigned c = s; c <= e; c++)
                charmap[c] = (uint16_t) (c + (unsigned) d);
    }
    return true;
fail:
    return false;
}

static bool getname(const Pgfont *font, unsigned id, char *buf) {
    section cursect = ((otf*) pg_get_font_impl(font))->name;

    BC(4, 2,            "NAME_TBL_HEADER");
    BC(6, 12 * PW(2),   "NAME_TBL_NRECORDS");
    BC(0, PW(4),        "NAME_TBL_STRINGS_OFF");

    unsigned best = 0;
    for (unsigned p = 6, end = 6 + 12 * PW(2); p < end; p += 12) {
        if (PW(p + 6) != id)
            continue;
        unsigned platform = PW(p);
        unsigned encoding = PW(p + 2);
        unsigned lang =     PW(p + 4);

        if (platform == 3 && encoding == 1)
            best = lang == 1033? p: (best? best: p); // Prefer English (en-US)
        else if (platform == 0 && encoding == 1)
            best = lang == 1033? p: (best? best: p); // Prefer English (en-US)
    }

    if (!best)
        goto fail;

    unsigned length =   PW(best + 8);
    unsigned offset =   PW(best + 10);
    unsigned src =      PW(4) + offset;
    BC(src, length, "NAME_TBL_DATA");
    for (unsigned i = 0; i < length / 2; i++)
        buf = (char*) pg_write_utf8((uint8_t*) buf, PW(src + i * 2));
    *buf = 0;
    return true;

fail:
    return false;
}

static void
ttoutline(Pg *g, Pgfont *font, Pgmat ctm, unsigned glyph) {

    bool        dw = OTF(font)->longloca;
    uint32_t    offset = dw? TD(loca, glyph * 4): TW(loca, glyph * 2) * 2;
    uint32_t    size = dw?  TD(loca, glyph * 4 + 4) - offset:
                            TW(loca, glyph * 2 + 2) * 2 - offset;
    if (size == 0)
        return;

    section     cursect = OTF(font)->glyf;
    BC(offset, 0,       "LOCA_GLYPH_OFF");
    BC(offset, size,    "LOCA_GLYPH_SIZE");

    cursect = (section) {cursect.ptr + offset, size};

    BC(8, 2, "GLYF_HEADER");
    unsigned ncontours = (unsigned) (int16_t) PW(0);

    // Simple Glyph.
    if ((signed) ncontours > 0) {
        BC(10 + 2 * ncontours, 2, "GLYF_CONTOUR_ENDS_SHORT");
        BC(10 + 2 * ncontours + 2, PW(10 + 2 * ncontours),
            "GLYF_CONTOUR_NINSTR");

        unsigned    ends = 10;
        unsigned    npoints = PW(10 + (ncontours - 1) * 2) + 1;
        unsigned    ninstr = PW(10 + 2 * ncontours);
        unsigned    flags = 10 + 2 * ncontours + 2 + ninstr;
        unsigned    fsize = 0;
        unsigned    xsize = 0;
        unsigned    ysize = 0;

        // Determine the length of the flags and x coordinates.
        for (unsigned n = 0; n < npoints; fsize++) {
            BC(flags + fsize, PB(flags + fsize) & 8? 1: 2, "GLYF_FLAG");

            uint8_t f = PB(flags + fsize);
            uint8_t rep = 1 + (f & 8? PB(flags + ++fsize): 0);

            xsize += rep * (f & 2? 1: f & 16? 0: 2);
            ysize += rep * (f & 4? 1: f & 32? 0: 2);
            n += rep;
        }
        BC(flags + fsize, xsize, "GLYF_XS");
        BC(flags + fsize + xsize, ysize, "GLYF_YS");

        // Unpack points.
        // We need to keep track of the position in the original coordinates
        // `(x, y)` since scaling deltas and adding it to `p` would not
        // have the same effect.
        float       x = 0.0f;
        float       y = 0.0f;
        Pgpt     home = pg_pt(0.0f, 0.0f);    // Start of this contour.
        Pgpt     oldp = home;                // Prev point; used in curves.
        Pgpt     p = home;                   // Current point.
        unsigned    xi = flags + fsize;         // X coordinate index.
        unsigned    yi = xi + xsize;            // Y coordinate index.
        bool        curving = false;            // Last segment was a curve.
        unsigned    next = 0;                   // Index of next contour.
        unsigned    ci = 0;                     // Contour index into `ends`.
        unsigned    fi = flags;                 // Flag index.

        for (unsigned n = 0; n < npoints; fi++) {
            uint8_t     f = PB(fi);
            uint8_t     rep = 1 + (f & 8? PB(++fi): 0);

            while (rep-- > 0) {
                bool    contourstart = n == next;
                bool    anchor = f & 1;

                n++;

                // Get the x and y delta for this segment.
                x += f & 2? (f & 16? PB(xi): -PB(xi)):
                            (f & 16? 0.0f: (int16_t) PW(xi));
                y += f & 4? (f & 32? PB(yi): -PB(yi)):
                            (f & 32? 0.0f: (int16_t) PW(yi));
                xi += f & 2? 1: f & 16? 0: 2;
                yi += f & 4? 1: f & 32? 0: 2;

                oldp = p;
                p = pg_apply_mat(ctm, pg_pt(x, y));

                if (contourstart) {                     // Closing subpath
                    next = PW(ends + ci++ * 2) + 1;
                    if (curving)
                        pg_curve3_to_pt(g, oldp, home);
                    pg_close_path(g);
                    pg_move_to_pt(g, p);
                    home = p;
                    curving = false;
                }
                else if (anchor && curving) {           // Curve-to-line
                    pg_curve3_to_pt(g, oldp, p);
                    curving = false;
                }
                else if (anchor)                        // Line-to-line
                    pg_line_to_pt(g, p);
                else if (curving) {                     // Line-to-curve
                    Pgpt m = pg_mid_pt(oldp, p);
                    pg_curve3_to_pt(g, oldp, m);
                }
                else                                    // Curve-to-curve
                    curving = true;
            }
        }
        if (npoints != 0) {
            if (curving)
                pg_curve3_to_pt(g, p, home);
            pg_close_path(g);
        }

    }

    // Composite Glyph.
    else if ((signed) ncontours < 0) {

        unsigned    p = 10;

        // Iterate over each component glyph.
        while (true) {
            BC(p, 4, "GLYF_COMPOSITE_HEADER");
            unsigned    flags = PW(p);
            unsigned    newglyph = PW(p + 2);
            Pgmat       tm = pg_ident_mat();
            p += 4;

            // Get x- and y-translation.
            if (~flags & 2) {
                // "Matching points" not supported.
                DBGMSG("GLYF_COMPOSITE_MATCH_POINT");
                p += flags & 1? 4: 2;
            }
            else if (flags & 1) {
                BC(p, 4, "GLYF_COMPOSITE_WORD_XLATE");
                tm.e = (int16_t) PW(p);
                tm.f = (int16_t) PW(p + 2);
                p += 4;
            }
            else {
                BC(p, 2, "GLYF_COMPOSITE_BYTE_XLATE");
                tm.e = (int8_t) PB(p);
                tm.f = (int8_t) PB(p + 1);
                p += 2;
            }

            // Get Scale
            if (flags & 0x08) {         // Has a single scale
                BC(p, 2, "GLYF_COMPOSITE_SINGLE_SCALE");
                tm.a = tm.d = (int16_t) PW(p) * (1.0f / (1 << 14));
                p += 2;
            } else if (flags & 0x40) {  // Separate x and y scale
                BC(p, 4, "GLYF_COMPOSITE_SEP_SCALE");
                tm.a = (int16_t) PW(p) * (1.0f / (1 << 14));
                tm.d = (int16_t) PW(p + 2) * (1.0f / (1 << 14));
                p += 4;
            } else if (flags & 0x80) {  // Supplying matrix
                BC(p, 8, "GLYF_COMPOSITE_MATRIX");
                tm.a = (int16_t) PW(p) * (1.0f / (1 << 14));
                tm.b = (int16_t) PW(p + 2) * (1.0f / (1 << 14));
                tm.c = (int16_t) PW(p + 4) * (1.0f / (1 << 14));
                tm.d = (int16_t) PW(p + 6) * (1.0f / (1 << 14));
                p += 8;
            }

            ttoutline(g, font, pg_mul_mat(tm, ctm), newglyph);

            if (~flags & 0x20)  // No more components
                break;
        }
    }
    return;

fail:
    return;
}


static inline Pgpt
rmove(Pg *g, Pgmat ctm, Pgpt a, Pgpt b) {
    b = pg_add_pts(a, b);
    pg_close_path(g);
    pg_move_to_pt(g, pg_apply_mat(ctm, b));
    DBGTRACE("        MOVE (%g, %g)\n", b.x, b.y);
    return b;
}

static inline Pgpt
rline(Pg *g, Pgmat ctm, Pgpt a, Pgpt b) {
    b = pg_add_pts(a, b);
    pg_line_to_pt(g, pg_apply_mat(ctm, b));
    DBGTRACE("        LINE (%g, %g) - (%g, %g)\n", a.x, a.y, b.x, b.y);
    return b;
}

static inline Pgpt
rcurve(Pg *g, Pgmat ctm,  Pgpt a, Pgpt b, Pgpt c, Pgpt d) {
    b = pg_add_pts(a, b);
    c = pg_add_pts(b, c);
    d = pg_add_pts(c, d);
    pg_curve4_to_pt(g,
        pg_apply_mat(ctm, b),
        pg_apply_mat(ctm, c),
        pg_apply_mat(ctm, d));
    DBGTRACE("        CURV (%g, %g) - (%g, %g) - (%g, %g) - (%g, %g)\n",
        a.x, a.y, b.x, b.y, c.x, c.y, d.x, d.y);
    return d;
}

static void cffoutline(Pg *g, Pgfont *font, Pgmat ctm, unsigned glyph) {
    typedef struct {
        section src;
        size_t  p;
    } frame;

    float       s[48];                              // Argument Stack.
    frame       fs[10];                             // Frame Stack (for subrs).
    cffindex    lsubrs = OTF(font)->lsubrs;     // Local Subroutines.
    cffindex    gsubrs = OTF(font)->gsubrs;     // Global Subroutines.
    section     cursect;                            // Glyph CharString.
    size_t      p = 0;                              // Cursor pointer.
    unsigned    n = 0;                              // Stack argument count.
    unsigned    nf = 0;                             // Stack frame count.
    Pgpt     a = {0.0f, 0.0f};
    unsigned    i;
    unsigned    hints = 0;
    float       given;
    bool        header = true;

    // Get the CharString
    if (!indexindex(OTF(font)->charstrings, glyph, &cursect))
        FAIL("CFF_GLYPH_MISSING_CHARSTRING");

    // Run the Type2 CharString program.
    while (p < cursect.size) {

        unsigned op = PB(p);

        // Operand.
        // Push on argument stack and go to next operand/operator.
        if (op == 28 || op >= 32) {

            if (n >= 48)
                FAIL("TYPE2_STACK_OVERFLOW");

            // 16.16 float operand
            if (op == 255) {
                BC(p, 5, "TYPE2_READ_FLOAT");
                uint32_t  num = (uint32_t) PB(p + 1) << 24 |
                                (uint32_t) PB(p + 2) << 16 |
                                (uint32_t) PB(p + 3) << 8  |
                                (uint32_t) PB(p + 4) << 0;
                s[n++] = (float) (int32_t) num * 1.0f / 65536.0f;
                p += 5;
                continue;
            }

            // Normal integer (mostly the same as CFF encoding; see function).
            s[n++] = (float) cffint(cursect, &p);
            continue;
        }

        // Two-byte Operator.
        else if (op == 12) {
            BC(p, 2, "TYPE2_TWO_BYTE_OP");
            op = 0x0c00 + PB(p + 1);
            p += 2;
        }

        // One-Byte Operator.
        else
            p++;

        #ifdef TRACE_FONT
            int pad = op > 31? DBGTRACE("(%d %d)", op >> 8, op & 255):
                              DBGTRACE("(%d)", op);
            for (pad = 8 - pad; pad-- > 0; ) DBGTRACE(" ");
            DBGTRACE("[%d]", n);
            for (unsigned i = 0; i < n; i++)
                DBGTRACE(" %g", s[i]);
            DBGTRACE("\n");
        #endif


        // Execute Operator.
        switch (op) {

        case 1:         // hstem
        case 3:         // vstem
        case 18:        // hstemhm
        case 23:        // vstemhm
            hints += n / 2;
            break;

        case 19:        // hintmask
        case 20:        // cntrmask
            if (header) // Optionally defined vsteams.
                hints += n / 2;
            header = false;
            BC(p + (hints + 7) / 8, 0, "TYPE2_MASK");
            p += (hints + 7) / 8;
            break;

        case 10:    // callsubr
        case 29:    // callgsubr
            if (n < 1)
                FAIL("TYPE2_SUBR_ARG");
            if (nf >= sizeof fs / sizeof *fs)
                FAIL("TYPE2_CALL_DEPTH");

            fs[nf++] = (frame) {cursect, p};

            {
                cffindex subrs = op == 29? gsubrs: lsubrs;
                unsigned biased = biassubr((unsigned) (int) s[n - 1], subrs.n);
                if (!indexindex(subrs, biased, &cursect))
                    FAIL("TYPE2_SUBR_NUM");
                p = 0;
            }
            n--;        // Remove subroutine number from the stack.
            continue;   // Leave arguments on the stack.

        // case 0:      // Reserved
        // case 1:      // hstem above
        // case 2:      // Reserved
        // case 3:      // vstem above

        case 4:         // vmoveto
            if (n < 1)
                FAIL("TYPE2_VMOVETO_ARG");
            a = rmove(g, ctm, a, pg_pt(0.0f, s[n - 1]));
            header = false;
            break;

        case 5:         // rlineto
            for (i = 0; i + 2 <= n; i += 2)
                a = rline(g, ctm, a, pg_pt(s[i], s[i + 1]));
            break;

        case 6:         // hlineto
            i = 0;
            if (n & 1)
                a = rline(g, ctm, a, pg_pt(s[i++], 0.0f));
            for ( ; i < n; i++) {
                Pgpt b = i & 1? pg_pt(0.0f, s[i]): pg_pt(s[i], 0.0f);
                a = rline(g, ctm, a, b);
            }
            break;

        case 7:         // vlineto
            i = 0;
            if (n & 1)
                a = rline(g, ctm, a, pg_pt(0.0f, s[i++]));
            for ( ; i < n; i++) {
                Pgpt b = i & 1? pg_pt(s[i], 0.0f): pg_pt(0.0f, s[i]);
                a = rline(g, ctm, a, b);
            }
            break;

        case 8:         // rrcurveto
            for (i = 0; i + 6 <= n; i += 6)
                a = rcurve(g, ctm, a,
                    pg_pt(s[i + 0], s[i + 1]),
                    pg_pt(s[i + 2], s[i + 3]),
                    pg_pt(s[i + 4], s[i + 5]));
            break;

        // case 9:      // Reserved
        // case 10:     // callsubr above

        case 11:        // return
            if (nf == 0)
                FAIL("TYPE2_RET_EMPTY");
            nf--;
            cursect = fs[nf].src;
            p = fs[nf].p;
            continue;   // Leave arguments on the stack.

        // case 12:     // escape
        // case 13:     // Reserved

        case 14:        // endchar
            pg_close_path(g);
            goto done;

        // case 15:     // Reserved
        // case 16:     // Reserved
        // case 17:     // Reserved
        // case 18:     // hstemhm above
        // case 19:     // hintmask above
        // case 20:     // cntrmask above

        case 21:        // rmoveto
            if (n < 2)
                FAIL("TYPE2_RMOVETO_ARGS");
            a = rmove(g, ctm, a, pg_pt(s[n - 2], s[n - 1]));
            header = false;
            break;

        case 22:        // hmoveto
            if (n < 1)
                FAIL("TYPE2_HMOVETO_ARG");
            a = rmove(g, ctm, a, pg_pt(s[n - 1], 0.0f));
            header = false;
            break;

        // case 23:     // vstemhm above

        case 24:        // rcurveline
            for (i = 0; i + 6 <= n; i += 6)
                a = rcurve(g, ctm, a,
                        pg_pt(s[i + 0], s[i + 1]),
                        pg_pt(s[i + 2], s[i + 3]),
                        pg_pt(s[i + 4], s[i + 5]));
            if (i != n - 2)
                FAIL("TYPE2_RCURVELINE_ARGS");
            a = rline(g, ctm, a, pg_pt(s[n - 2], s[n - 1]));
            break;

        case 25:        // rlinecurve
            for (i = 0; i + 6 < n; i += 2) // Stop six away from end.
                a = rline(g, ctm, a, pg_pt(s[i + 0], s[i + 1]));
            if (i != n - 6)
                FAIL("TYPE2_RLINECURVE_ARGS");
            a = rcurve(g, ctm, a,
                pg_pt(s[n - 6], s[n - 5]),
                pg_pt(s[n - 4], s[n - 3]),
                pg_pt(s[n - 2], s[n - 1]));
            break;

        case 26:        // vvcurveto
            i = 0;
            given = n & 1? s[i++]: 0.0f;
            for ( ; i + 4 <= n; i += 4) {
                a = rcurve(g, ctm, a,
                        pg_pt(given, s[i + 0]),
                        pg_pt(s[i + 1], s[i + 2]),
                        pg_pt(0.0f, s[i + 3]));
                given = 0.0f;
            }
            break;

        case 27:        // hhcruveto
            i = 0;
            given = n & 1? s[i++]: 0.0f;
            for ( ; i + 4 <= n; i += 4) {
                a = rcurve(g, ctm, a,
                    pg_pt(s[i + 0], given),
                    pg_pt(s[i + 1], s[i + 2]),
                    pg_pt(s[i + 3], 0.0f));
                given = 0.0f;
            }
            break;

        // case 28:     // shortint operand above
        // case 29:     // callgsubr above

        case 30:        // vhcurveto
            for (i = 0; i + 4 <= n; i += 4) {
                a = rcurve(g, ctm, a,
                        pg_pt(0.0f, s[i + 0]),
                        pg_pt(s[i + 1], s[i + 2]),
                        pg_pt(s[i + 3], n - i == 5? s[i + 4]: 0.0f));
                i += 4;
                if (i + 4 <= n)
                    a = rcurve(g, ctm, a,
                        pg_pt(s[i + 0], 0.0f),
                        pg_pt(s[i + 1], s[i + 2]),
                        pg_pt(n - i == 5? s[i + 4]: 0.0f, s[i + 3]));
            }
            break;

        case 31:        // hvcurveto
            for (i = 0; i + 4 <= n; i += 4) {
                a = rcurve(g, ctm, a,
                    pg_pt(s[i + 0], 0.0f),
                    pg_pt(s[i + 1], s[i + 2]),
                    pg_pt(n - i == 5? s[i + 4]: 0.0f, s[i + 3]));
                i += 4;
                if (i + 4 <= n)
                    a = rcurve(g, ctm, a,
                        pg_pt(0.0f, s[i + 0]),
                        pg_pt(s[i + 1], s[i + 2]),
                        pg_pt(s[i + 3], n - i == 5? s[i + 4]: 0.0f));
            }
            break;

        default:
            DBGTRACE("TYPE2_OPERATOR");
            break;
        }

        // Most operators clear the stack.
        n = 0;
    }

done:
    pg_close_path(g);
    return;

fail:
    DBGTRACE("FAILED");
    return;
}


static void *_init(Pgfont *font, const uint8_t *data, size_t filesize, unsigned index) {

    if (!data || !filesize)
        return 0;

    section         full = {data, filesize};
    section         cursect = {data, filesize};
    unsigned        nfonts = 0;

    // Check if this is a TrueType Collection.
    uint32_t        header = 0;
    BC(header + 12, 4, "TTC_HEADER");
    if (PD(0) == C4('t','t','c','f')) {
        unsigned ver = PD(4);
        nfonts = PD(8);

        if (ver != 0x10000 && ver != 0x20000)
            FAIL("TTC_VER");
        if (nfonts <= index)
            FAIL("TTC_INDEX");

        header = PD(12 + 4 * index);
    }

    // Check sfnt Header.
    BC(header + 10, 2, "SFNT_HEADER");
    uint32_t        signature = PD(header);
    unsigned        ntables = PW(header + 4);
    if (signature != 0x10000 &&
        signature != C4('O','T','T','O') &&
        signature != C4('t','r','u','e') &&
        signature != C4('t','y','p','1'))
    {
        FAIL("SFNT_SIGNATURE");
    }

    // Note required Table Record Entry.
    section     cmap = {0, 0};
    section     glyf = {0, 0};
    section     head = {0, 0};
    section     hhea = {0, 0};
    section     hmtx = {0, 0};
    section     maxp = {0, 0};
    section     name = {0, 0};
    section     os2 = {0, 0};
    section     post = {0, 0};
    section     loca = {0, 0};
    section     cff  = {0, 0};
    section     cff2 = {0, 0};
    for (unsigned i = 0; i < ntables; i++) {
        uint64_t    p = header + 12 + i * 16;
        BC(p + 12, 4, "TBL_RECORD");
        uint32_t    tag = PD(p);
        uint32_t    offset = PD(p + 8);
        uint32_t    size = PD(p + 12);
        section     sect = {data + offset, size};

        BOUNDS(full, offset, 0, "TBL_REC_OFF");
        BOUNDS(full, offset, size, "TBL_REC_SIZE");

        switch (tag) {
        case C4('C','F','F',' '):   cff  = sect; break;
        case C4('C','F','F','2'):   cff2 = sect; break;
        case C4('c','m','a','p'):   cmap = sect; break;
        case C4('g','l','y','f'):   glyf = sect; break;
        case C4('h','e','a','d'):   head = sect; break;
        case C4('h','h','e','a'):   hhea = sect; break;
        case C4('h','m','t','x'):   hmtx = sect; break;
        case C4('l','o','c','a'):   loca = sect; break;
        case C4('m','a','x','p'):   maxp = sect; break;
        case C4('n','a','m','e'):   name = sect; break;
        case C4('O','S','/','2'):   os2  = sect; break;
        case C4('p','o','s','t'):   post = sect; break;
        }
    }

    unsigned    cffver = cff.ptr ? 1:
                         cff2.ptr? 2:
                         0;

    if (!cmap.ptr) FAIL("NO_CMAP_TBL");
    if (!head.ptr) FAIL("NO_HEAD_TBL");
    if (!hhea.ptr) FAIL("NO_HHEA_TBL");
    if (!hmtx.ptr) FAIL("NO_HMTX_TBL");
    if (!maxp.ptr) FAIL("NO_MAXP_TBL");
    if (!name.ptr) FAIL("NO_NAME_TBL");
    if (!os2.ptr)  FAIL("NO_OS2_TBL");
    if (!post.ptr) FAIL("NO_POST_TBL");

    // Required TrueType tables.
    if (cffver == 0) {
        if (!loca.ptr) FAIL("NO_LOCA_TBL");
        if (!glyf.ptr) FAIL("NO_GLYF_TBL");
    }

    // HEAD
    cursect = head;
    float   units;
    bool    italic;
    bool    longloca;
    {
        if (cursect.size != 54)     FAIL("HEAD_TBL_SIZE");
        if (PD(0) != 0x10000)       FAIL("HEAD_TBL_VER");
        if (PD(12) != 0x5F0F3CF5)   FAIL("HEAD_TBL_SIGNATURE");
        if (PW(52) != 0)            FAIL("GLYF_DATA_FORMAT");

        units = PW(18);
        italic = PW(44) & 2;
        longloca = PW(50);
    }

    // HHEA
    unsigned    nhmtx;
    cursect = hhea;
    {
        if (cursect.size != 36)     FAIL("HHEA_TBL_SIZE");
        if (PW(32) != 0)            FAIL("HHEA_TBL_VER");
        nhmtx = PW(34);
    }

    // MAXP
    unsigned    nglyphs;
    cursect = maxp;
    {
        BC(0, 4, "MAXP_TBL_SIZE");
        if (cffver) {
            if (PD(0) != 0x5000)    FAIL("MAXP_TBL_CFF");
            if (cursect.size != 6)  FAIL("MAXP_TBL_SIZE");
        }
        else {
            if (PD(0) != 0x10000)   FAIL("MAXP_TBL_VER");
            if (cursect.size != 32) FAIL("MAXP_TBL_SIZE");
        }
        nglyphs = PW(4);
    }

    // HMTX
    if (hmtx.size != 4 * nhmtx + (nglyphs - nhmtx) * 2)
        FAIL("HMTX_TBL_SIZE");

    // OS/2
    cursect = os2;
    BC(0, 2, "OS2_TBL_SIZE");
    if (PW(0) == 0? os2.size != 78:
        PW(0) == 1? os2.size != 86:
        PW(0) == 2? os2.size != 96:
        PW(0) == 3? os2.size != 96:
        PW(0) == 4? os2.size != 96:
        PW(0) == 5? os2.size != 100:
        os2.size < 100) // Allow newer versions but meet minimum of version 5
    {
        FAIL("OS2_TBL_VER_SIZE");
    }

    // POST
    cursect = post;
    BC(0, 2, "POST_TBL_SIZE");
    if (PD(0) == 0x10000? post.size != 32:
        PD(0) == 0x20000? post.size < 34:
        PD(0) == 0x25000? post.size < 34:
        PD(0) == 0x30000? post.size != 32: true)
    {
        FAIL("POST_TBL_VER_SIZE");
    }
    bool fixed = PD(12);
    float angle = (float) (int32_t) PD(4);

    // LOCA
    if (cffver == 0) {
        if (loca.size != (nglyphs + 1) * (longloca? 4: 2))
            FAIL("LOCA_TBL_SIZE");
    }

    // CFF
    cffindex    gsubrs = {0, 0, 0, 0};
    cffindex    lsubrs = {0, 0, 0, 0};
    cffindex    charstrings = {0, 0, 0, 0};
    cursect = cff;
    if (cffver == 1) {
        cffindex        tmp;
        section         top = {0, 0};
        section         private = {0, 0};
        size_t          csoffset = 0;
        size_t          lsoffset = 0;
        cffoperand      s[48];
        unsigned        n;

        // Header.
        BC(3, 1, "CFF_TBL_SIZE");
        if (PW(0) != 0x100)
            FAIL("CFF_VER");

        uint8_t     abs = PB(3);        // Size of Offsets
        size_t      p = PB(2);          // Header Size.

        if (abs > 4)
            FAIL("CFF_ABS_OFF_SIZE");
        if (p < 4)
            FAIL("CFF_HEADER_SIZE");

        if (!makeindex(cursect, &p, &tmp))  FAIL("CFF_NAME_INDEX");
        if (tmp.n != 1)                     FAIL("CFF_MULTIPLE_FONTS");
        if (!makeindex(cursect, &p, &tmp))  FAIL("CFF_TOP_DICT_INDEX");
        if (tmp.n != 1)                     FAIL("CFF_MULTIPLE_FONTS");
        if (!indexindex(tmp, 0, &top))      FAIL("CFF_TOP_DICT");
        if (!makeindex(cursect, &p, &tmp))  FAIL("CFF_STRING_INDEX");
        if (!makeindex(cursect, &p, &gsubrs)) FAIL("CFF_GSUBR_INDEX");

        // Top DICT.
        cursect = top;
        p = 0;
        while (p < cursect.size)
            switch (nextdict(cursect, &p, s, &n)) {
            case -1:                // Error (diagnostic already printed).
                goto fail;

            case 17:                // CharStrings offset.
                if (n != 1)        FAIL("CFF_TOP_DICT_CSOFF");
                csoffset = (size_t) (int) s[0].i;
                break;

            case 18:                // Private DICT
                if (n != 2)        FAIL("CFF_TOP_DICT_PRIVATE");
                BOUNDS(cff, (int) s[1].i, 0, "CFF_PRIVATE_OFF");
                BOUNDS(cff, (int) s[1].i, (int) s[0].i, "CFF_PRIVATE_SIZE");
                private = (section) {cff.ptr + (int) s[1].i, (size_t) (int) s[0].i};
                break;

            case 0x0c25:            // FDSelect
                // Should not be in an OpenType/CFF.
                FAIL("CFF_TOP_DICT_FDSELECT");
            }

        if (!csoffset)
            FAIL("CFF_NO_CHARSTRINGS");

        // Private DICT.
        if (private.ptr) {
            cursect = private;
            p = 0;
            while (p < cursect.size)
                switch (nextdict(cursect, &p, s, &n)) {
                case -1:
                    goto fail;
                case 19:                // Local subrs offset.
                    if (n != 1)
                        FAIL("CFF_PRIVATE_LSUBR_ARG");
                    lsoffset = (size_t) (cursect.ptr - full.ptr)
                                + (size_t) (int) s[0].i;
                    break;
                }
        }

        // Locate CharStrings.
        if (!makeindex(cff, &csoffset, &charstrings))
            FAIL("CFF_CHARSTRINGS_INDEX");
        if (charstrings.n != nglyphs)
            FAIL("CFF_CHARSTRINGS_NGLYPHS");

        // Locate Local Subrs if present.
        if (lsoffset && !makeindex(full, &lsoffset, &lsubrs))
            FAIL("CFF_LSUBR_INDEX");
    }

    // CMAP
    uint16_t charmap[65536] = {0};
    cursect = cmap;
    {
        bool    bmploaded = false;

        BC(0, 4, "CMAP_TBL_SIZE");
        if (PW(0) != 0)
            FAIL("CMAP_TBL_VER");
        BC(4, PW(2) * 8, "CMAP_TBL_NSUBTBL");

        // Find a table that can be decoded.
        for (unsigned i = 0; i < PW(2); i++) {
            unsigned rec = 4 + 8 * i;

            // Windows / Unicode-BMP subtable.
            if (PW(rec) == 3 && PW(rec + 2) == 1) {
                unsigned    off = PD(rec + 4);
                section     subtable = {cursect.ptr + off, cursect.size - off};
                BOUNDS(subtable, 0, 2, "CMAP_SUBTBL_SIZE");
                if (!loadcmap4(charmap, subtable))
                    goto fail;
                bmploaded = true;
                break;
            }
        }

        if (!bmploaded)
            FAIL("CMAP_NO_FORMAT");
    }

    pg_init_font(font, units, nglyphs, charmap);

    return new(otf,
        .cffver = cffver,
        .longloca = longloca,
        .nhmtx = nhmtx,
        .lsubrs = lsubrs,
        .gsubrs = gsubrs,
        .charstrings = charstrings,
        .glyf = glyf,
        .hmtx = hmtx,
        .loca = loca,
        .name = name,
        .os2 = os2,
        .fontindex = index,
        .nfonts = nfonts,
        .angle = angle,
        .fixed = fixed,
        .italic = italic);

fail:
    return 0;
}

static void _free(Pgfont *font) {
    (void) font;
}

static float _propf(Pgfont *font, Pgfont_prop id) {
    float sx = pg_get_font_scale(font).x;
    float sy = pg_get_font_scale(font).y;
    switch (id) {
    case PG_FONT_FORMAT:        return 0;
    case PG_FONT_INDEX:         return (float) OTF(font)->fontindex;
    case PG_FONT_NFONTS:        return (float) OTF(font)->nfonts;
    case PG_FONT_FIXED:         return (float) OTF(font)->fixed;
    case PG_FONT_ITALIC:        return (float) OTF(font)->italic;
    case PG_FONT_FAMILY:        return 0.0f;
    case PG_FONT_STYLE:         return 0.0f;
    case PG_FONT_FULL_NAME:     return 0.0f;
    case PG_FONT_WEIGHT:        return TW(os2, 4);
    case PG_FONT_WIDTH_CLASS:   return TW(os2, 6);
    case PG_FONT_ANGLE:         return OTF(font)->angle;
    case PG_FONT_PANOSE:        return 0.0f;
    case PG_FONT_NGLYPHS:       return (float) pg_get_nglyphs(font);
    case PG_FONT_EM:            return sy * pg_get_em_units(font);
    case PG_FONT_AVG_WIDTH:     return sx * (int16_t) TW(os2, 2);
    case PG_FONT_ASCENDER:      return sy * (int16_t) TW(os2, 68);
    case PG_FONT_DESCENDER:     return sy * (int16_t) TW(os2, 70);
    case PG_FONT_LINEGAP:       return sy * (int16_t) TW(os2, 72);
    case PG_FONT_XHEIGHT:       return TW(os2, 0) >= 2? sy * (int16_t) TW(os2, 86): 0;
    case PG_FONT_CAPHEIGHT:     return TW(os2, 0) >= 2? sy * (int16_t) TW(os2, 88): 0;
    case PG_FONT_SUB_SX:        return sy * (int16_t) TW(os2, 10);
    case PG_FONT_SUB_SY:        return sy * (int16_t) TW(os2, 12);
    case PG_FONT_SUB_X:         return sy * (int16_t) TW(os2, 14);
    case PG_FONT_SUB_Y:         return sy * (int16_t) TW(os2, 16);
    case PG_FONT_SUP_SX:        return sy * (int16_t) TW(os2, 18);
    case PG_FONT_SUP_SY:        return sy * (int16_t) TW(os2, 20);
    case PG_FONT_SUP_X:         return sy * (int16_t) TW(os2, 22);
    case PG_FONT_SUP_Y:         return sy * (int16_t) TW(os2, 24);
    }
    return 0.0f;
}

static const char *_props(Pgfont *font, Pgfont_prop id) {
    char *buf = OTF(font)->prop_buf;
    switch (id) {
    case PG_FONT_FORMAT:        return  OTF(font)->cffver? "CFF": "TTF";
    case PG_FONT_FAMILY:        return  getname(font, 16, buf)? buf:
                                        getname(font, 1, buf)? buf:
                                        "";
    case PG_FONT_STYLE:         return  getname(font, 17, buf)? buf:
                                        getname(font, 2, buf)? buf:
                                        "";
    case PG_FONT_FULL_NAME:     return  getname(font, 4, buf)? buf:
                                        getname(font, 3, buf)? buf:
                                        "";
    case PG_FONT_FIXED:         return pg_font_prop_int(font, id)
                                        ? "Fixed Pitched":
                                        "Proportional";
    case PG_FONT_ITALIC:        return pg_font_prop_int(font, id)
                                        ? "Italic":
                                        "Roman";
    case PG_FONT_WEIGHT:
        return weights[TW(os2, 4) < 1000? TW(os2, 4) / 100: 0];
    case PG_FONT_WIDTH_CLASS:
        return widths[TW(os2, 6) < 1000? TW(os2, 6) / 100: 0];
    case PG_FONT_PANOSE:
        {
            const uint8_t *panose;
            panose = OTF(font)->os2.ptr + 32;
            sprintf(buf,
                "%02d %02d %02d %02d %02d %02d %02d %02d %02d %02d",
                panose[0], panose[1], panose[2], panose[3], panose[4],
                panose[5], panose[6], panose[7], panose[8], panose[9]);
            return buf;
        }
    case PG_FONT_INDEX:
    case PG_FONT_NFONTS:
    case PG_FONT_ANGLE:
    case PG_FONT_NGLYPHS:
    case PG_FONT_EM:
    case PG_FONT_AVG_WIDTH:
    case PG_FONT_ASCENDER:
    case PG_FONT_DESCENDER:
    case PG_FONT_LINEGAP:
    case PG_FONT_XHEIGHT:
    case PG_FONT_CAPHEIGHT:
    case PG_FONT_SUB_X:
    case PG_FONT_SUB_Y:
    case PG_FONT_SUB_SX:
    case PG_FONT_SUB_SY:
    case PG_FONT_SUP_X:
    case PG_FONT_SUP_Y:
    case PG_FONT_SUP_SX:
    case PG_FONT_SUP_SY:
        sprintf(buf, "%g", pg_font_prop_float(font, id));
        return buf;
    }
    return "";
}

static void _glyph_path(Pg *g, Pgfont *font, Pgpt at, unsigned glyph) {
    Pgpt   s = pg_get_font_scale(font);

    if (s.x != 0.0f && s.y != 0.0f) {
        Pgmat ctm = { s.x, 0.0f, 0.0f, -s.y, at.x, at.y };

        if (OTF(font)->cffver == 0)
            ttoutline(g, font, ctm, glyph);

        else if (OTF(font)->cffver == 1)
            cffoutline(g, font, ctm, glyph);
    }
}

static Pgpt _measure_glyph(Pgfont *font, unsigned glyph) {
    unsigned    nhmtx = OTF(font)->nhmtx;
    Pgpt       s = pg_get_font_scale(font);
    float       adv = TW(hmtx, 4 * (glyph < nhmtx? glyph: nhmtx - 1)) * s.y;

    return pg_pt(adv, pg_get_font_height(font));
}

static const Pgfont_methods methods = {
    _init,
    _free,
    _propf,
    _props,
    _glyph_path,
    _measure_glyph,
};

Pgfont *pg_open_otf_font(const uint8_t *data, size_t size, unsigned index) {
    return pg_new_font(&methods, data, size, index);
}
