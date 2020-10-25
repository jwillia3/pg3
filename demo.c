#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pg.h>
#include <pgutil.h>

GLFWwindow  *win;
Pg          *g;
char        buffer[256];

Pgpaint     bg;
Pgpaint     accent;
Pgpaint     fg;

unsigned    startfont;


static void update();

static void resized(GLFWwindow *win, int width, int height) {
    (void) win;
    glViewport(0, 0, width, height);
    pgresize(g, width, height);
    update();
}

static void
key_callback(GLFWwindow *win, int key, int scancode, int action, int mods) {
    (void) win;
    (void) scancode;
    (void) action;
    (void) mods;
    if (key == GLFW_KEY_ESCAPE || (key == 'W' && mods & GLFW_MOD_CONTROL))
        exit(0);

    if (action > 0) {
        if (key == GLFW_KEY_UP) {
            startfont = startfont? startfont - 1: 0;
            update();
        } else if (key == GLFW_KEY_DOWN) {
            startfont += 1;
            update();
        }
    }

    if (action > 0 && (isalnum(key) || key == ' ')) {
        strcat(buffer, (char[]){(char) key, 0});
        update();
    }
}

void openwindow(int width, int height, char *title) {
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 16);
    glfwWindowHint(GLFW_DOUBLEBUFFER, true);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_STENCIL_BITS, 8);

    win = glfwCreateWindow(width, height, title, 0, 0);
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, resized);
    glfwSetWindowRefreshCallback(win, update);
    glfwSetKeyCallback(win, key_callback);

    glewInit();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    g = pgopengl_canvas(width, height);
}

void fonts() {
    unsigned cols = 5;
    float   big = 18.0f * 96.0f / 72.0f;
    float   small = 14.0f * 96.0f / 72.0f;
    float   colwid = pgwidth(g) / (float) cols;


    // Draw each block.
    Pgpt     p = pgpt(0.0f, 0.0f);
    unsigned    c = 0;
    float       rowsize = 0.0f;
    Pgfamily    *all = pglist_fonts();
    Pgfamily    *fam = all;

    for ( ; fam->name && fam != all + startfont * cols; fam++);

    for ( ; fam->name; fam++) {

        if (p.y >= pgheight(g))
            break;

        // Move to next line.
        if (c % cols == 0) {
            p = pgpt(0.0f, p.y + rowsize);
            rowsize = 0.0f;
            for (Pgfamily *f = fam, *end = f + cols; f != end && f->name; f++)
                rowsize = fmaxf(rowsize, big + small * (float) f->nfaces);
            rowsize += big * 2;
        }

        float top = p.y;
        Pgfont *fbig = pgfind_font(fam->name, 900, false);
        pgscale_font(fbig, big, 0.0f);
        pgset_fill(g, accent);
        pgdraw_string(g, fbig, p, fam->name);
        pgfree_font(fbig);
        pgfill(g);
        p.y += big;

        pgset_fill(g, fg);
        for (Pgface *fac = fam->faces; fac->family; fac++) {
            Pgfont *fsmall = pgopen_font_file(fac->path, fac->index);
            pgscale_font(fsmall, small, 0.0f);
            pgdraw_string(g, fsmall, p, fac->style);
            pgfree_font(fsmall);
            p.y += small;
        }
        pgfill(g);

        p = pgpt(p.x + colwid, top);
        c++;
    }
}

void allglyphs(Pgfont *font) {
    unsigned n = (unsigned) pgfontpropi(font, PG_FONT_NGLYPHS);
    float em = sqrtf(pgwidth(g) * pgheight(g) / (float) n);
    em = sqrtf(pgwidth(g) * (pgheight(g) - em) / (float) n);
    pgscale_font(font, em, em);
    Pgpt     p = pgpt(0.0f, 0.0f);
    for (unsigned i = 0; i < n; i++) {
        if (p.x + em >= pgwidth(g))
            p = pgpt(0.0f, p.y + em);
        pgdraw_glyph(g, font, p, i);
        pgfill(g);
        p.x += em;
    }
}

void star(Pgpt p, float size, float points) {
    float f = size; // full size
    float h = size * 0.5f; // half size
    for (float i = 0; i <= points; i++) {
        float tf = 2.0f * 3.14159f * i / points - 3.14159f / 2.0f;
        float th = 2.0f * 3.14159f * (i + 0.5f) / points - 3.14159f / 2.0f;
        if (i == points)
            pgclose(g);
        else {
            if (i == 0)
                pgmove(g, pgaddpt(p, pgpt(cosf(tf) * f, sinf(tf) * f)));
            else
                pgline(g, pgaddpt(p, pgpt(cosf(tf) * f, sinf(tf) * f)));
            pgline(g, pgaddpt(p, pgpt(cosf(th) * h, sinf(th) * h)));
        }
    }
}

void flag() {
    pgsave(g);
    pgset_fill_color(g, (Pgcolor) {0.125f, 0.3f, 0.09f, 1.0f});
    pgset_stroke_color(g, (Pgcolor) {0.125f, 0.3f, 0.65f, 1.0f});
    pgset_line_width(g, 10.0f);
    pgtranslatef(g, pgwidth(g) / 2.0f - 300.0f, pgheight(g) / 2.0f - 25.0f);
    star(pgpt(000, 0), 50, 6);
    star(pgpt(200, 0), 50, 6);
    star(pgpt(400, 0), 50, 6);
    star(pgpt(600, 0), 50, 6);
    pgfillstroke(g);
    pgrestore(g);
}

void typing() {
    float fontsz = 14.0f * 96.0f / 72.0f;

    Pgfont *font = pgfind_font("Arial", 300, 0);
    pgscale_font(font, fontsz, 0.0f);
    for (float y = 10; y < pgheight(g); y += fontsz) {
        pgdraw_string(g, font, pgpt(10, y), buffer);
        pgfill(g);
    }
    pgfree_font(font);
}

static void update() {
    pgset_default_colorspace(PG_LCHAB);
    bg = pglinearf(0.0f, 0.0f, pgwidth(g), pgheight(g));
    pgadd_stop(&bg, 0.0f, (Pgcolor) {1.0f, 0.125f, 0.25f, 1.0f});
    pgadd_stop(&bg, 1.0f, (Pgcolor) {0.7f, 0.125f, 0.25f, 1.0f});
    accent = pgsolidf(0.0f, 0.5f, 0.65f, 1.0f);
    fg = pgsolidf(0.1f, 0.0f, 0.0f, 1.0f);

    pgident(g);
    pgclear(g, bg);

    // typing();
    // flag();
    // allglyphs(pgfind_font("Arial Nova", 0, 0));
    fonts();

    // Pgpaint grad = pglinearf(0.0f, 0.0f, pgwidth(g), 0.0f);
    // pgadd_stop(&grad, 0.0f, (Pgcolor){0.125f, 0.5f, 0.0f, 1.0f});
    // pgadd_stop(&grad, 1.0f, (Pgcolor){0.125f, 0.5f, 1.0f, 1.0f});
    // pgset_fill(g, grad);
    // pgclear(g, grad);

    glfwSwapBuffers(win);
}

int main(int argc, char **argv) {
    (void) argc;
    (void) argv;
    setvbuf(stdout, 0, _IONBF, 0);
    openwindow(1920, 1080, "Demo");
    while (!glfwWindowShouldClose(win))
        glfwWaitEvents();
}
