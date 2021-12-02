#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pg3.h>

enum XmlType {
    XML_ERROR=-1,
    XML_END,
    XML_OPEN_TAG,
    XML_END_OPEN_TAG,
    XML_ATTR,
    XML_QUOTED,
    XML_SELF_CLOSE,
    XML_CLOSE_TAG,
};

enum XmlMode {
    XML_NORMAL,
    XML_OPENED,
};


const char      *xml_data;
const char      *xmlp;
char            xml_tag[128];
char            xml_attr[128];
char            xml_combined[128 + 128];
enum XmlMode    xml_mode;
const char      *xml_value_start;
const char      *xml_value_end;


void
xml_open(const char *text)
{
    xml_data = strdup(text);
    xmlp = xml_data;
}

void
xml_free(void)
{
    free((void*) xml_data);
    xml_data = 0;
    xmlp = 0;
}

void
xml_location(int *line, int *column)
{
    int l = 1;
    int c = 1;
    for (const char *i = xml_data; i < xmlp; i++)
        if (*i == '\n') {
            l++;
            c = 1;
        } else
            c++;
    *line = l;
    *column = c;
}

char*
xml_tag_name(void)
{
    return xml_tag;
}

char*
xml_attr_name(void)
{
    return xml_attr;
}

char*
xml_combined_name(void)
{
    sprintf(xml_combined, "%s.%s", xml_tag, xml_attr);
    return xml_combined;
}

static
bool
xml_starts(char *with)
{
    unsigned i;
    for (i = 0; with[i]; i++)
        if (with[i] != xmlp[i])
            return false;
    xmlp += i;
    return true;
}

static
int
xml_read_name(char *buf)
{
    int n = 0;
    while (*xmlp && (isalnum(*xmlp) || strchr(":?!_", *xmlp)))
        buf[n++] = *xmlp++;
    buf[n] = 0;
    return n;
}

bool
xml_read_quote(uint32_t quote)
{
    xml_value_start = xmlp;
    while (*xmlp && *xmlp != quote)
        xmlp++;
    if (*xmlp != quote)
        return false;
    xml_value_end = xmlp;
    xmlp++;
    return true;
}

char*
xml_value_copy(void)
{
    size_t n = xml_value_end - xml_value_start;
    char *buf = malloc(n + 1);
    memcpy(buf, xml_value_start, n);
    buf[n] = 0;
    return buf;
}

void
xml_skip_space(void)
{
    more:
    while (isspace(*xmlp))
        xmlp++;
    if (xml_starts("<!--")) {
        while (*xmlp && !xml_starts("-->"))
            xmlp++;
        goto more;
    }
}

static
enum XmlType
xml_next_normal(void)
{
    xml_skip_space();
    if (*xmlp == 0)
        return XML_END;
    if (xml_starts("</")) {
        xml_read_name(xml_tag);
        xml_skip_space();
        if (!xml_starts(">"))
            return XML_ERROR;
        return XML_CLOSE_TAG;
    }
    if (xml_starts("<")) {
        xml_read_name(xml_tag);
        xml_mode = XML_OPENED;
        return XML_OPEN_TAG;
    }
    if (xml_starts(">")) {
        xml_mode = XML_NORMAL;
        return XML_END_OPEN_TAG;
    }
    return XML_ERROR;
}

static
enum XmlType
xml_next_opened(void)
{
    xml_skip_space();

    if (xml_starts(">") || xml_starts("?>") || xml_starts("!>")) {
        xml_mode = XML_NORMAL;
        return XML_CLOSE_TAG;
    }

    if (xml_starts("/>")) {
        xml_mode = XML_NORMAL;
        return XML_SELF_CLOSE;
    }

    if (xml_read_name(xml_attr)) {
        xml_skip_space();
        if (xml_starts("=")) {
            xml_skip_space();
            if (*xmlp == '"' || *xmlp == '\'') {
                if (!xml_read_quote(*xmlp++))
                    return XML_ERROR;
                return XML_ATTR;
            }
            else
                return XML_ERROR;
        } else {
            xml_value_start = xmlp;
            xml_value_end = xmlp;
        }
        return XML_ATTR;
    }
    return XML_ERROR;
}

enum XmlType
xml_next(void)
{
    switch (xml_mode) {
    case XML_NORMAL:
        return xml_next_normal();

    case XML_OPENED:
        return xml_next_opened();
    }
}

PgPath*
svgdata(PgPath *path, const char *data)
{
    static uint8_t map[256] = {
        [' ']=1, ['\t']=1, ['\r']=1, ['\n']=1, ['\v']=1, [',']=1,
        ['M']=2, ['m']=2,
        ['Z']=2, ['z']=2,
        ['L']=2, ['l']=2,
        ['H']=2, ['h']=2,
        ['V']=2, ['v']=2,
        ['C']=2, ['c']=2,
        ['S']=2, ['s']=2,
        ['Q']=2, ['q']=2,
        ['T']=2, ['t']=2,
        ['+']=3, ['-']=3,
        ['0']=3, ['1']=3, ['2']=3, ['3']=3, ['4']=3,
        ['5']=3, ['6']=3, ['7']=3, ['8']=3, ['9']=3,
    };
    static uint8_t required[256] = {
        ['M']=2, ['m']=2,
        ['Z']=0, ['z']=0,
        ['L']=2, ['l']=2,
        ['H']=1, ['h']=1,
        ['V']=1, ['v']=1,
        ['C']=6, ['c']=6,
        ['S']=4, ['s']=4,
        ['Q']=4, ['q']=4,
        ['T']=2, ['t']=2,
    };

    const char  *s = data;
    unsigned    cmd = 0;
    float       x[8];
    unsigned    n = 0;
    PgPt        a = {0, 0};
    PgPt        r = {0, 0};

    if (path)
        pg_path_move(path, 0, 0);
    else
        path = pg_path();

    while (*s) {
        switch (map[*(unsigned char*)s]) {
        default:    // error: bad character.
            s++;
            break;
        case 1:     // space.
            s++;
            break;
        case 2:     // command.
            n = 0;
            cmd = *s++;
            break;
        case 3:     // number.
            x[n++ & 7] = strtod(s, (char**)&s);
            break;
        }

        if (required[cmd] == n) {
            switch (cmd) {
            case 'M':   pg_path_move(path, x[0], x[1]);
                        a = PgPt(x[0], x[1]);
                        break;
            case 'm':   pg_path_rmove(path, x[0], x[1]);
                        a = PgPt(a.x + x[0], a.y + x[1]);
                        break;
            case 'Z':   pg_path_close(path);
                        break;
            case 'z':   pg_path_close(path);
                        break;
            case 'L':   pg_path_line(path, x[0], x[1]);
                        a = PgPt(x[0], x[1]);
                        break;
            case 'l':   pg_path_rline(path, x[0], x[1]);
                        a = PgPt(a.x + x[0], a.y + x[1]);
                        break;
            case 'H':   pg_path_line(path, x[0], a.y);
                        a.x = x[0];
                        break;
            case 'h':   pg_path_rline(path, x[0], 0);
                        a.x += x[0];
                        break;
            case 'V':   pg_path_line(path, a.x, x[0]);
                        a.y = x[0];
                        break;
            case 'v':   pg_path_rline(path, 0, x[0]);
                        a.y += x[0];
                        break;

            case 'C':   curve4:
                        pg_path_curve4(path, x[0], x[1], x[2], x[3], x[4], x[5]);
                        a = PgPt(x[4], x[5]);
                        r = PgPt(x[4] - x[2], x[5] - x[3]);
                        break;

            case 'c':   for (int i=0; i<6; i+=2) {
                            x[i] += a.x;
                            x[i + 1] += a.y;
                        }
                        goto curve4;

            case 'S':   smooth4:
                        pg_path_curve4(path,
                                  a.x + r.x, a.y + r.y,
                                  x[0], x[1],
                                  x[2], x[3]);
                        a = PgPt(x[2], x[3]);
                        r = PgPt(x[2] - x[0], x[3] - x[1]);
                        break;

            case 's':   for (int i=0; i<6; i+=2) {
                            x[i] += a.x;
                            x[i+1] += a.y;
                        }
                        goto smooth4;

            case 'Q':   curve3:
                        pg_path_curve3(path, x[0], x[1], x[2], x[3]);
                        a = PgPt(x[2], x[3]);
                        r = PgPt(x[2] - x[0], x[3] - x[1]);
                        break;

            case 'q':   for (int i=0; i<4; i+=2) {
                            x[i] += a.x;
                            x[i + 1] += a.y;
                        }
                        goto curve3;

            case 'T':   smooth3:
                        r = PgPt(a.x + r.x, a.y + r.y);
                        pg_path_curve3(path, a.x + r.x, a.y + r.y, x[0],x[1]);
                        a = PgPt(x[0], x[1]);
                        r = PgPt(a.x - r.x, a.y - r.y);
                        break;

            case 't':   for (int i=0; i<4; i+=2) {
                            x[i] += a.x;
                            x[i + 1] += a.y;
                        }
                        goto smooth3;
            break;
            }
            n = 0;
        }
    }

    return path;
}

float       scale = 5;
PgPt        trans = {0,1000};
Pg          *g;
bool        which=1;



char akzidenz_text[65536];
char tiger_text[128*1024];

#define P(...) {}
// #define P(...) printf(__VA_ARGS__)

void
read_tiger(Pg *g)
{
    if (!*tiger_text) {
        FILE *file = fopen("tiger.svg", "r");
        if (!file)
            puts("cannot open tiger.svg"), exit(1);
        tiger_text[fread(tiger_text, 1, sizeof tiger_text, file)] = 0;
        fclose(file);
    }

    pg_clear(g);
    pg_identity(g);

    PgPaint fill = pg_solid(PG_SRGB, 0, 0, 0, 0);
    PgPaint stroke = pg_solid(PG_SRGB, 0, 0, 0, 0);
    bool    do_fill = false;
    bool    do_stroke = false;

    xml_open(tiger_text);
    int ln,cl;
    while (true) {

        switch (xml_next()) {
        case XML_ERROR:
        default:
            xml_location(&ln, &cl);
            P("xml error: tiger.svg:%d:%d\n", ln, cl);
            exit(0);

        case XML_END:
            goto done;

        case XML_OPEN_TAG:
            P("<%s", xml_tag_name());
            if (!strcmp("path", xml_tag_name())) {
                pg_save(g);
                do_stroke = false;
                do_fill = true;
            }
            break;

        case XML_END_OPEN_TAG:
            P(">\n");
            if (!strcmp("g", xml_tag_name())) {
                pg_save(g);
            }
            break;

        case XML_ATTR:
            P(" %s=...", xml_attr_name());

            if (!strcmp("path.style", xml_combined_name())) {
                char    *tmp = xml_value_copy();
                char    *s = tmp;

                while (*s) {
                    char *name = s;
                    while (*s && *s != ':')
                        s++;
                    if (!*s)
                        break;
                    *s = 0;

                    char *value = ++s;
                    while (*s && *s != ';')
                        s++;
                    if (*s)
                        *s++ = 0;
                    else
                        *s = 0;

                    if (!strcmp(name, "fill")) {
                        if (*value == '#') {
                            uint32_t rgb = strtoul(value + 1, 0, 16);
                            fill = pg_solid(PG_SRGB,
                                            (rgb >> 16 & 255) / 255.0f,
                                            (rgb >> 8 & 255) / 255.0f,
                                            (rgb >> 0 & 255) / 255.0f,
                                            1);
                            pg_set_fill(g, &fill);
                            do_fill = true;
                        }
                        else if (!strcmp(value, "none"))
                            do_fill = false;
                    }
                    else if (!strcmp(name, "stroke")) {
                        if (*value == '#') {
                            uint32_t rgb = strtoul(value + 1, 0, 16);
                            stroke = pg_solid(PG_SRGB,
                                            (rgb >> 16 & 255) / 255.0f,
                                            (rgb >> 8 & 255) / 255.0f,
                                            (rgb >> 0 & 255) / 255.0f,
                                            1);
                            pg_set_stroke(g, &stroke);
                            do_stroke = true;
                        }
                        else if (!strcmp(value, "none"))
                            do_stroke = false;
                    }
                    else if (!strcmp(name, "stroke-width")) {
                        float w = strtod(value, 0);
                        pg_set_line_width(g, w);
                        do_stroke = true;
                    }
                    // else
                    //     printf("%s %s\n", name, value);
                }
                free(tmp);
            }

            if (!strcmp("path.d", xml_combined_name())) {
                char    *tmp = xml_value_copy();
                PgPath  *d = svgdata(0, tmp);
                pg_append(g, d);
                pg_path_free(d);
                free(tmp);
            }

            if (!strcmp("g.transform", xml_combined_name())) {
                char    *tmp = xml_value_copy();
                float   a, b, c, d, e, f;

                if (2 == sscanf(tmp, "translate(%f %f)", &e, &f))
                    pg_translate(g, e, f);
                else if (1 == sscanf(tmp, "scale(%f)", &a))
                    pg_scale(g, a, a);
                else if (6 == sscanf(tmp, "matrix(%f %f %f %f %f %f)",
                                     &a, &b, &c, &d, &e, &f))
                    pg_set_tm(g, (PgTM) { a, b, c, d, e, f });
                else
                    puts(tmp);
                pg_scale(g, scale, scale);
                pg_translate(g, 0, (249 - 48.64) * scale);
                pg_translate(g, trans.x, trans.y);

                free(tmp);
            }

            break;

        case XML_SELF_CLOSE:
            P("/>\n");

            if (!strcmp("path", xml_tag_name())) {
                if (do_stroke && do_fill)
                    pg_fill_stroke(g);
                else if (do_stroke)
                    pg_stroke(g);
                else if (do_fill)
                    pg_fill(g);
                pg_restore(g);
            }
            break;

        case XML_CLOSE_TAG:
            P("</>\n");
            if (!strcmp("g", xml_tag_name())) {
                pg_restore(g);
            }
            break;
        }
    }

    done:
    xml_free();
}

void
read_akzidenz(void)
{
    static PgPath *path;
    static PgPaint fg;

    if (!*akzidenz_text) {
        FILE *file = fopen("demo.data", "r");
        if (!file)
            puts("cannot open demo.data"), exit(1);
        akzidenz_text[fread(akzidenz_text, 1, sizeof akzidenz_text, file)] = 0;
        fclose(file);

        path = svgdata(0, akzidenz_text);
        fg = pg_solid(PG_LCHAB, .125, .25, .75, 1);
    }

    pg_clear(g);
    pg_identity(g);
    pg_scale(g, scale, scale);
    pg_append(g, path);
    pg_set_fill(g, &fg);
    pg_fill(g);
}

#include <time.h>
#include <GL/gl.h>

float flattening;
float stenciling;
float mapping;
unsigned segments;

void
redraw(void) {
    glFinish();
    flattening = 0;
    stenciling = 0;
    mapping = 0;
    segments = 0;

    struct timespec a,b;
    clock_gettime(CLOCK_MONOTONIC, &a);

    if (which)
        read_tiger(g);
    else
        read_akzidenz();
    pg_update();

    glFinish();
    clock_gettime(CLOCK_MONOTONIC, &b);
    uint64_t ans = a.tv_sec * 1000000000 + a.tv_nsec;
    uint64_t bns = b.tv_sec * 1000000000 + b.tv_nsec;
    puts("\n");
    printf("%12.8f ms (total)\n", (bns - ans) / 1000000.0);
    printf("%12.8f ms (flattening)\n", flattening / 1000000.0);
    printf("%12.8f ms (stenciling)\n", stenciling / 1000000.0);
    printf("%12.8f ms (mapping)\n", mapping / 1000000.0);
    printf("%d segments\n", segments);
}

int
main()
{
    setvbuf(stdout, 0, _IONBF, 0);


    read_akzidenz();



    g = pg_window(1024, 768, "Demo");
    redraw();

    while (pg_event()) {
        if (pg_mouse_wheel()) {
            float ss = pg_mod_keys() & PG_MOD_CTRL? 1: .1f;
            scale = fmaxf(.01f, scale + pg_mouse_wheel() * ss);
            redraw();
        }
        if (pg_key()) {
            float ss = pg_mod_keys() & PG_MOD_CTRL? 10: 1;
            if (pg_key() == PG_KEY_DOWN) {
                trans.y += ss;
                redraw();
            }
            if (pg_key() == PG_KEY_UP) {
                trans.y -= ss;
                redraw();
            }
            if (pg_key() == PG_KEY_LEFT) {
                trans.x -= ss;
                redraw();
            }
            if (pg_key() == PG_KEY_RIGHT) {
                trans.x += ss;
                redraw();
            }
        }


        if (pg_key() == 'x') {
            extern int algorithm;
            algorithm = !algorithm;
            redraw();
        }
        else if (pg_key() == 'q' || pg_key() == PG_KEY_ESCAPE)
            break;
    }
}
