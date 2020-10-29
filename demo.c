#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pg.h>

GLFWwindow  *win;
Pg          *g;

Pgpaint     bg;
Pgpaint     accent;
Pgpaint     fg;

unsigned    startfont;


static void update();

static void resized(GLFWwindow *win, int width, int height) {
    (void) win;
    glViewport(0, 0, width, height);
    pg_resize_canvas(g, (unsigned) width, (unsigned) height);
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
    g = pg_opengl_canvas((unsigned) width, (unsigned) height);
}

void fonts() {
    unsigned cols = 5;
    float   big = 18.0f * 96.0f / 72.0f;
    float   small = 14.0f * 96.0f / 72.0f;
    float   colwid = pg_get_size(g).x / (float) cols;


    // Draw each block.
    Pgpt     p = pg_pt(0.0f, 0.0f);
    unsigned    c = 0;
    float       rowsize = 0.0f;
    Pgfamily    *all = pg_list_fonts();
    Pgfamily    *fam = all;

    for ( ; fam->name && fam != all + startfont * cols; fam++);

    for ( ; fam->name; fam++) {

        if (p.y >= pg_get_size(g).y)
            break;

        // Move to next line.
        if (c % cols == 0) {
            p = pg_pt(0.0f, p.y + rowsize);
            rowsize = 0.0f;
            for (Pgfamily *f = fam, *end = f + cols; f != end && f->name; f++)
                rowsize = fmaxf(rowsize, big + small * (float) f->nfaces);
            rowsize += big * 2;
        }

        float top = p.y;

        Pgfont *fbig = pg_find_font(fam->name, 900, false);
        pg_scale_font(fbig, big, 0.0f);

        pg_set_underline(g, true);
        pg_string_path(g, fbig, p, fam->name);
        pg_set_underline(g, false);
        pg_set_fill(g, accent);
        pg_fill(g);

        pg_free_font(fbig);
        p.y += big;

        pg_set_fill(g, fg);
        for (Pgface *fac = fam->faces; fac->family; fac++) {
            Pgfont *fsmall = pg_open_font_file(fac->path, fac->index);
            pg_scale_font(fsmall, small, 0.0f);

            pg_string_path(g, fsmall, p, fac->style);
            p.y += small;

            pg_free_font(fsmall);
        }
        pg_fill(g);

        p = pg_pt(p.x + colwid, top);
        c++;
    }
}

void allglyphs(Pgfont *font) {
    unsigned n = (unsigned) pg_font_prop_int(font, PG_FONT_NGLYPHS);
    float em = sqrtf(pg_get_size(g).x * pg_get_size(g).y / (float) n);
    em = sqrtf(pg_get_size(g).x * (pg_get_size(g).y - em) / (float) n);
    pg_scale_font(font, em, em);
    Pgpt     p = pg_pt(0.0f, 0.0f);
    for (unsigned i = 0; i < n; i++) {
        if (p.x + em >= pg_get_size(g).x)
            p = pg_pt(0.0f, p.y + em);
        pg_glyph_path(g, font, p, i);
        pg_fill(g);
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
            pg_close_path(g);
        else {
            if (i == 0)
                pg_move_to_pt(g, pg_add_pts(p, pg_pt(cosf(tf) * f, sinf(tf) * f)));
            else
                pg_line_to_pt(g, pg_add_pts(p, pg_pt(cosf(tf) * f, sinf(tf) * f)));
            pg_line_to_pt(g, pg_add_pts(p, pg_pt(cosf(th) * h, sinf(th) * h)));
        }
    }
}

void flag() {
    pg_save(g);
    pg_set_fill(g, pg_solid(PG_LCHAB, 0.125f, 0.3f, 0.09f, 1.0f));
    pg_set_stroke(g, pg_solid(PG_LCHAB, 0.125f, 0.3f, 0.65f, 1.0f));
    pg_set_line_width(g, 10.0f);
    pg_ctm_translate(g, pg_get_size(g).x / 2.0f - 300.0f, pg_get_size(g).y / 2.0f - 25.0f);
    star(pg_pt(000, 0), 50, 6);
    star(pg_pt(200, 0), 50, 6);
    star(pg_pt(400, 0), 50, 6);
    star(pg_pt(600, 0), 50, 6);
    pg_fill_stroke(g);
    pg_restore(g);
}

static void update() {

    bg = pg_linear(PG_LCHAB, 0.0f, 0.0f, 0.0f, pg_get_size(g).y);
    pg_add_stop(&bg, 0.0f, 0.6f, 0.125f, 0.275f, 1.0f);
    pg_add_stop(&bg, 1.0f, 0.9f, 0.125f, 0.275f, 1.0f);
    accent = pg_solid(PG_LCHAB, 0.0f, 0.5f, 0.65f, 1.0f);
    fg = pg_solid(PG_LCHAB, 0.1f, 0.0f, 0.0f, 1.0f);

    pg_ctm_identity(g);
    pg_clear(g, bg);

    // typing();
    // flag();
    // allglyphs(pg_find_font("URW Bookman", 300, 0));
    fonts();

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
