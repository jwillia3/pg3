static
inline
size_t
pg_utf8_nbytes(uint32_t codepoint)
{
    if (codepoint < 0x80)
        return 1;

    else if (codepoint < 0x0800)
        return 2;

    else if (codepoint < 0x10000)
        return 3;

    else if (codepoint < 0x10ffff)
        return 4;

    return 0;
}


static
inline
size_t
pg_utf8_following(uint8_t lead)
{
    return  lead < 0x80? 0:
            lead < 0xd0? 1:
            lead < 0xf0? 2:
                         3;
}


static
inline
uint32_t
pg_read_utf8(const char **stringp, const char *limitp)
{
    const uint8_t   *in = (const uint8_t*) *stringp;
    const uint8_t   *limit = (const uint8_t*) limitp;

    uint32_t    c = *in++;
    size_t      nbytes =    c < 0x80? 0:
                            c < 0xd0? 1:
                            c < 0xf0? 2:
                                      3;
    uint32_t    min =       c < 0x80? 0x00:
                            c < 0xd0? 0x80:
                            c < 0xf0? 0x800:
                                      0x10000;
    uint32_t    mask =      c < 0x80? 0x7f:
                            c < 0xd0? 0x07ff:
                            c < 0xf0? 0xffff:
                                      0x10ffff;
    unsigned    got = 0;

    while (in < limit && (*in & 0xc0) == 0x80) {
        c = (c << 6) + (*in++ & 0x3f);
        got++;
    }

    *stringp = (const char*) in;

    if (got != nbytes)
        return 0xfffd;

    if (c < min)
        return 0xfffd;

    return c & mask;
}


static
inline
const char*
pg_utf8_start(const char *str, const char *start, const char *limit)
{
    if (str >= limit)
        return limit;

    if (str < start)
        return start;

    while (str > start && (*str & 0xc0) == 0x80)
        str--;

    return str;
}


static
inline
uint32_t
pg_rev_read_utf8(const char **in, const char *start, const char *limit)
{
    *in = pg_utf8_start(*in, start, limit);
    return pg_read_utf8((const char*[]) { *in }, limit);
}


static
inline
char*
pg_write_utf8(char *outp, char *limitp, uint32_t c)
{
    uint8_t *out = (uint8_t*) outp;
    uint8_t *limit = (uint8_t*) limitp;

    if (c < 0x80) {
        if (out < limit)
            *out++ = (uint8_t) c;
    }
    else if (c < 0x0800) {
        if (out + 2 < limit) {
            *out++ = (uint8_t) (0xc0 + (c >> 6));
            *out++ = (uint8_t) (0x80 + (c & 0x3f));
        }
    }

    else if (c < 0x10000) {
        if (out + 3 < limit) {
            *out++ = (uint8_t) (0xe0 + (c >> 12));
            *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
            *out++ = (uint8_t) (0x80 + (c & 0x3f));
        }
    }

    else if (c < 0x10ffff) {
        if (out + 4 < limit) {
            *out++ = (uint8_t) (0xe0 + (c >> 18));
            *out++ = (uint8_t) (0x80 + (c >> 12 & 0x3f));
            *out++ = (uint8_t) (0x80 + (c >> 6 & 0x3f));
            *out++ = (uint8_t) (0x80 + (c & 0x3f));
        }
    }

    return (char*) out;
}


static
inline
int
pg_stricmp(const char *a, const char *b)
{
    while (*a && *b) {
        uint32_t ca = tolower(pg_read_utf8(&a, a + 4));
        uint32_t cb = tolower(pg_read_utf8(&b, b + 4));

        if (ca != cb)
            return ca < cb? -1: 1;
    }
    return *a? 1: *b? -1: 0;
}


static
inline
const char*
pg_stristr(const char *ap, const char *bp)
{
    const char *next = ap;

    while (*next) {
        const char *b = bp;
        const char *a = next;

        while (*b) {
            uint32_t ca = tolower(pg_read_utf8(&a, a + 4));
            uint32_t cb = tolower(pg_read_utf8(&b, b + 4));

            if (ca != cb)
                break;
        }

        if (!*b)
            return (const char*) next;

        pg_read_utf8(&next, next + 4);
    }

    return 0;
}
