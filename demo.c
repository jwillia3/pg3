#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pg3.h>

GLFWwindow  *win;
Pg          *g;

PgPaint     accent;
PgPaint     fg;

unsigned    startfont;


static void update();

static void resized(GLFWwindow *win, int width, int height) {
    (void) win;
    glViewport(0, 0, width, height);
    pg_resize(g, (float) width, (float) height);
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
    g = pg_opengl((float) width, (float) height);
}

void fonts() {
    unsigned cols = 5;
    float   big = 18.0f * 96.0f / 72.0f;
    float   small = 14.0f * 96.0f / 72.0f;
    float   colwid = g->size.x / (float) cols;


    // Draw each block.
    PgPt     p = PgPt(0.0f, 0.0f);
    unsigned    c = 0;
    float       rowsize = 0.0f;
    PgFamily    *all = pg_list_fonts();
    PgFamily    *fam = all;

    for ( ; fam->name && fam != all + startfont * cols; fam++);

    for ( ; fam->name; fam++) {

        if (p.y >= g->size.y)
            break;

        // Move to next line.
        if (c % cols == 0) {
            p = PgPt(0.0f, p.y + rowsize);
            rowsize = 0.0f;
            for (PgFamily *f = fam, *end = f + cols; f != end && f->name; f++)
                rowsize = fmaxf(rowsize, big + small * (float) f->nfaces);
            rowsize += big * 2;
        }

        float top = p.y;

        PgFont *fbig = pg_find_font(fam->name, 900, false);
        pg_scale_font(fbig, big, 0.0f);

        g->s.underline = true;
        pg_string_path(g, fbig, p.x, p.y, fam->name);

        g->s.underline = false;
        g->s.fill = &accent;
        pg_fill(g);

        pg_free_font(fbig);
        p.y += big;

        g->s.fill = &fg;
        for (PgFace *fac = fam->faces; fac->family; fac++) {
            PgFont *fsmall = pg_open_font_file(fac->path, fac->index);
            pg_scale_font(fsmall, small, 0.0f);
            pg_printf(g, fsmall, p.x, p.y, "%s", fac->style);
            p.y += small;

            pg_free_font(fsmall);
        }
        pg_fill(g);

        p = PgPt(p.x + colwid, top);
        c++;
    }
}

static void update() {

    PgPaint bg = pg_linear(PG_LCHAB, 0.0f, 0.0f, g->size.x, g->size.y);
    pg_add_stop(&bg, 0.0f, 0.7f, 0.0f, 0.0f, 1.0f);
    pg_add_stop(&bg, 0.2f, 0.9f, 0.0f, 0.0f, 1.0f);
    pg_add_stop(&bg, 0.8f, 0.9f, 0.0f, 0.0f, 1.0f);
    pg_add_stop(&bg, 1.0f, 0.7f, 0.0f, 0.0f, 1.0f);

    accent = pg_solid(PG_LCHAB, 0.0f, 0.5f, 0.65f, 1.0f);
    fg = pg_solid(PG_LCHAB, 0.1f, 0.0f, 0.0f, 1.0f);


    pg_identity(g);

    g->s.clear = &bg;
    pg_clear(g);

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
