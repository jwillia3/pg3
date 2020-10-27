#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <pg.h>
#include <pg.internal.h>

#define BEZIER_LIMIT    10
#define CLOSED          65536
#define SUB(N)          ((N) & (CLOSED - 1))

typedef struct {
    GLuint      prog, vsh, fsh;
    GLint       posloc, ctmloc;
} GL;


static const char *VERTEX_SHADER[] = {
    "#version 110",
    "uniform mat3 ctm;",
    "attribute vec2 pos;",
    "void main() {",
    "   vec3 p = ctm * vec3(pos, 1.0);",
    "   gl_Position = vec4(p.x, p.y, 0.0, 1.0);",
    "}",
    0
};

static const char *FRAGMENT_SHADER[] = {
    "#version 110",
    "uniform int     cspace;",
    "uniform int     type;",
    "uniform vec2    a;",
    "uniform vec2    b;",
    "uniform float   ra;",
    "uniform float   rb;",
    "uniform float   stops[8];",
    "uniform vec4    colours[8];",
    "uniform int     nstops;",
    "",
    "vec4 lchtolab(vec4 lch) {",
    "    float l = lch.x;",
    "    float c = lch.y;",
    "    float h = lch.z * 2.0 * 3.14159;",
    "    return vec4(l, c * cos(h), c * sin(h), lch.w);",
    "}",
    "vec4 labtoxyz(vec4 lab) {",
    "    float l = lab.x;",
    "    float y = (l + 16.0) / 116.0;",
    "    float x = y + lab.y / 500.0;",
    "    float z = y - lab.z / 200.0;",
    "    x = x > 24.0 / 116.0? x*x*x: (x - 16.0/116.0) * 108.0/841.0;",
    "    y = y > 24.0 / 116.0? y*y*y: (y - 16.0/116.0) * 108.0/841.0;",
    "    z = z > 24.0 / 116.0? z*z*z: (z - 16.0/116.0) * 108.0/841.0;",
    "    return vec4(x * 950.4, y * 1000.0, z * 1088.8, lab.w);",
    "}",
    "vec4 xyztorgb(vec4 xyz) {",
    "    const mat3 m = mat3( 3.2404542, -0.9692660, 0.0556434,",
    "                        -1.5371385, 1.8760108, -0.2040259,",
    "                        -0.4985314, 0.0415560,  1.0572252);",
    "    return clamp(vec4(m * vec3(xyz), xyz.w), 0.0, 1.0);",
    "}",
    "vec4 gamma(vec4 rgb) {",
    "    float r = rgb.x;",
    "    float g = rgb.y;",
    "    float b = rgb.z;",
    "    return vec4(r < 0.0031308? 12.92 * r: 1.055 * pow(r, 1.0 / 2.40) - 0.055,",
    "                g < 0.0031308? 12.92 * g: 1.055 * pow(g, 1.0 / 2.40) - 0.055,",
    "                b < 0.0031308? 12.92 * b: 1.055 * pow(b, 1.0 / 2.40) - 0.055,",
    "                rgb.w);",
    "}",
    "vec4 convert(vec4 colour) {",
    "    return  cspace == 0? gamma(colour):",
    "            cspace == 1? gamma(xyztorgb(labtoxyz(lchtolab(colour)))):",
    "            cspace == 2? gamma(xyztorgb(labtoxyz(colour))):",
    "            cspace == 3? gamma(xyztorgb(colour)):",
    "            gamma(colour);",
    "}",
    "vec4 stopcolour(float t) {",
    "    int i;",
    "    for (i = 0; i < nstops && t > stops[i]; i++) {}",
    "    vec4 colour =   i == 0? colours[0]:",
    "                    i == nstops? colours[nstops - 1]:",
    "    mix(colours[i - 1],",
    "        colours[i],",
    "        (t - stops[i - 1]) / (stops[i] - stops[i - 1]));",
    "    return convert(colour);",
    "}",
    "void main() {",
    "    if (nstops == 1)",
    "        gl_FragColor = convert(colours[0]);",
    "    else if (type == 2) // Radial gradient.",
    "        gl_FragColor = stopcolour(length(vec2(gl_FragCoord) - a) / (rb - ra));",
    "    else { // Linear gradient.",
    "        vec2 dv = b - a;",
    "        vec2 dp = vec2(gl_FragCoord) - a;",
    "        float t = dot(dv, dp) / dot(dv, dv);",
    "        gl_FragColor = stopcolour(t);",
    "    }",
    "}",
    0
};

static inline Pgpt perp(Pgpt p) {
    return pg_pt(-p.y, p.x);
}

static GLuint make_buffer(GLenum target, const void *data, size_t size) {
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(target, buf);
    glBufferData(target, (GLsizei) size, data, GL_STREAM_DRAW);
    return buf;
}

static GLuint make_shader(GLenum type, const GLchar **srcarray) {
    char    *src = 0;
    GLsizei size = 0;
    for (unsigned i = 0; srcarray[i]; i++) {
        size_t s = strlen(srcarray[i]);
        src = realloc(src, (size_t) size + 1 + s + 1);
        memcpy(src + size, srcarray[i], s);
        size += s + 1;
        src[size - 1] = '\n';
    }
    src[size] = 0;

    GLuint  shader = glCreateShader(type);
    GLint   ok;

    glShaderSource(shader, 1, (const char**) &src, &size);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        GLchar  tmp[4096];
        GLsizei n;
        glGetShaderInfoLog(shader, sizeof tmp, &n, tmp);
        fprintf(stderr, "error:\n%s.\n", tmp);
        exit(1);
    }

    free(src);
    return shader;
}

static GLuint make_program(GLuint vshader, GLuint fshader) {
    GLint   ok;
    GLuint  program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (!ok) {
        GLchar  buf[4096];
        GLsizei n;
        glGetProgramInfoLog(program, sizeof buf, &n, buf);
        fprintf(stderr, "error linking:\n%s.\n", buf);
        exit(1);
    }
    return program;
}

static void set_coords(Pg *g) {
    GL *gl = pg_get_canvas_impl(g);
    Pgpt sz = pg_get_size(g);

    glUseProgram(gl->prog);

    Pgrect  r = pg_get_clip(g);
    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint) r.p.x,
              (GLint) (sz.y - r.p.y - r.size.y),
              (GLsizei) r.size.x,
              (GLsizei) r.size.y);

    float ctm[] = { 2.0f / sz.x, 0.0f, 0.0f,
                    0.0f, -2.0f / sz.y, 0.0f,
                    -1.0f, 1.0f, 0.0f };
    glUniformMatrix3fv(gl->ctmloc, 1, false, ctm);
}

static void set_paint(Pg *g, Pgpaint paint) {
    GL      *gl = pg_get_canvas_impl(g);
    GLuint  prog = gl->prog;

    glUniform1i(glGetUniformLocation(prog, "nstops"), paint.nstops);

    for (unsigned i = 0; i < paint.nstops; i++) {
        char tmp[16];
        snprintf(tmp, sizeof tmp, "colours[%d]", i);
        glUniform4f(glGetUniformLocation(prog, tmp),
            paint.colours[i].x,
            paint.colours[i].y,
            paint.colours[i].z,
            paint.colours[i].a);

        snprintf(tmp, sizeof tmp, "stops[%d]", i);
        glUniform1f(glGetUniformLocation(prog, tmp), paint.stops[i]);
    }

    float h = pg_get_size(g).y;
    glUniform1i(glGetUniformLocation(prog, "type"), paint.type);
    glUniform1i(glGetUniformLocation(prog, "cspace"), paint.cspace);
    glUniform2f(glGetUniformLocation(prog, "a"), paint.a.x, h - paint.a.y);
    glUniform2f(glGetUniformLocation(prog, "b"), paint.b.x, h - paint.b.y);
    glUniform1f(glGetUniformLocation(prog, "ra"), paint.ra);
    glUniform1f(glGetUniformLocation(prog, "rb"), paint.rb);
}

static void *_init(Pg *g) {
    (void) g;
    GLuint  vsh = make_shader(GL_VERTEX_SHADER, VERTEX_SHADER);
    GLuint  fsh = make_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    GLuint  prog = make_program(vsh, fsh);
    GLint   posloc = glGetAttribLocation(prog, "pos");
    GLint   ctmloc = glGetUniformLocation(prog, "ctm");
    return new(GL, prog, vsh, fsh, posloc, ctmloc);
}

static void _free(Pg *g) {
    GL *gl = pg_get_canvas_impl(g);
    glDeleteShader(gl->vsh);
    glDeleteShader(gl->fsh);
    glDeleteProgram(gl->prog);
    free(gl);
}

static void _clear(Pg *g, Pgpaint paint) {
    set_coords(g);
    if (paint.nstops == 1) {
        Pgcolor c = pg_convert_color(paint.cspace, paint.colours[0]);
        glClearColor(c.x, c.y, c.z, c.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    } else {
        GL      *gl = pg_get_canvas_impl(g);
        Pgpt    sz = pg_get_size(g);

        set_paint(g, paint);

        GLfloat verts[] = { 0.0f, 0.0f,
                            sz.x, 0.0f,
                            0.0f, sz.y,
                            sz.x, sz.y };

        GLuint quads = make_buffer(GL_ARRAY_BUFFER, verts, 8 * sizeof *verts);
        glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(gl->posloc);
        glDisable(GL_STENCIL_TEST);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
        glEnable(GL_STENCIL_TEST);
        glDisableVertexAttribArray(gl->posloc);
        glDeleteBuffers(1, &quads);

        glClear(GL_STENCIL_BUFFER_BIT);
    }
}

/*
    Flatten path into line segments.
    If `sub[i]` is the start of a subpath, `sub[i+1]` is the exclusive end.
    `sub[nsubs]` holds the total number of vertices.
    If a path is closed, the `CLOSED` bit is set on the start index.
*/
static void
flatten(Pg *g,
        Pgpt **pverts,
        unsigned *pnverts,
        unsigned **psubs,
        unsigned *pnsubs)
{
    Pgpt        *verts = malloc(65536 * sizeof *verts);
    unsigned    *subs = malloc(65536 * sizeof *subs);
    unsigned    nverts = 0;
    unsigned    nsubs = 0;
    Pgmat       ctm = pg_get_ctm(g);
    Pgpt        home = pg_apply_mat(ctm, pg_pt(0.0f, 0.0f));
    Pgpt        cur = home;
    Pgpath      path = pg_get_path(g);

    for (unsigned i = 0; i < path.nparts; i++) {
        if (nverts >= 65536 - (1 << BEZIER_LIMIT))
            break;

        Pgpt    *pts = path.parts[i].pt;
        switch (path.parts[i].form) {
        case PG_PART_MOVE:
            cur = home = verts[nverts++] = pg_apply_mat(ctm, pts[0]);
            subs[nsubs++] = nverts - 1;
            break;
        case PG_PART_LINE:
            cur = verts[nverts++] = pg_apply_mat(ctm, pts[0]);
            break;
        case PG_PART_CURVE3:
            nverts += flatten3(verts + nverts,
                            cur,
                            pg_apply_mat(ctm, pts[0]),
                            pg_apply_mat(ctm, pts[1]),
                            pg_get_flatness(g) * 0.5f,
                            BEZIER_LIMIT);
            cur = pg_apply_mat(ctm, pts[1]);
            break;
        case PG_PART_CURVE4:
            nverts += flatten4(verts + nverts,
                            cur,
                            pg_apply_mat(ctm, pts[0]),
                            pg_apply_mat(ctm, pts[1]),
                            pg_apply_mat(ctm, pts[2]),
                            pg_get_flatness(g) * 0.5f,
                            BEZIER_LIMIT);
            cur = pg_apply_mat(ctm, pts[2]);
            break;
        case PG_PART_CLOSE:
            verts[nverts++] = cur = home;
            if (nsubs)
                subs[nsubs - 1] |= CLOSED;
            break;
        }
    }
    subs[nsubs] = nverts;
    *pverts = verts;
    *pnverts = nverts;
    *psubs = subs;
    *pnsubs = nsubs;
}

static void _fill(Pg *g) {
    GL          *gl = pg_get_canvas_impl(g);
    Pgpt        *verts;
    unsigned    *subs;
    unsigned    nverts;
    unsigned    nsubs;

    flatten(g, &verts, &nverts, &subs, &nsubs);
    if (!nverts) {
        free(verts);
        free(subs);
        return;
    }

    set_coords(g);
    set_paint(g, pg_get_fill(g));

    /*
        Draw shape to stencil buffer by fanning triangles from a single vertex.
        For even-odd fill mode, all the triangles flip the bits under them.
        For non-zero winding mode, triangles going counter-clockwise increment
        the stencil, and clockwise decrement.
        For each, non-zero stencil values are drawn.
    */
    glColorMask(0, 0, 0, 0);
    glEnable(GL_STENCIL_TEST);
    glStencilFunc(GL_ALWAYS, 0, 0);

    GLuint src = make_buffer(GL_ARRAY_BUFFER, verts, nverts * sizeof *verts);
    glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(gl->posloc);

    if (pg_get_fill_rule(g) == PG_EVEN_ODD_RULE) {
        glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);
        for (unsigned i = 0; i < nsubs; i++) {
            int     start = SUB(subs[i]);
            int     n = SUB(subs[i + 1]) - SUB(subs[i]);
            glDrawArrays(GL_TRIANGLE_FAN, start, n);
        }
    } else {
        glEnable(GL_CULL_FACE);
        for (unsigned i = 0; i < nsubs; i++) {
            unsigned    start = SUB(subs[i]);
            unsigned    n = SUB(subs[i + 1]) - SUB(subs[i]);

            glCullFace(GL_FRONT);
            glStencilOp(GL_INCR_WRAP, GL_INCR_WRAP, GL_INCR_WRAP);
            glDrawArrays(GL_TRIANGLE_FAN, (GLint) start, (GLint) n);

            glCullFace(GL_BACK);
            glStencilOp(GL_DECR_WRAP, GL_DECR_WRAP, GL_DECR_WRAP);
            glDrawArrays(GL_TRIANGLE_FAN, (GLint) start, (GLint) n);
        }
        glDisable(GL_CULL_FACE);
    }

    glDisableVertexAttribArray(gl->posloc);
    glDeleteBuffers(1, &src);
    glColorMask(1, 1, 1, 1);

    // Draw a quad over mask only placing pixels where the stencil bit is set.
    Pgpt min = verts[0];
    Pgpt max = verts[0];
    for (unsigned i = 0; i < nverts; i++) {
        min.x = fminf(min.x, verts[i].x);
        min.y = fminf(min.y, verts[i].y);
        max.x = fmaxf(max.x, verts[i].x);
        max.y = fmaxf(max.y, verts[i].y);
    }
    GLfloat quadverts[] = {min.x, min.y,
                           max.x, min.y,
                           min.x, max.y,
                           max.x, max.y};
    GLuint quads = make_buffer(GL_ARRAY_BUFFER, quadverts, 8 * sizeof *quadverts);
    glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(gl->posloc);
    glStencilOp(GL_ZERO, GL_ZERO, GL_ZERO);
    glStencilFunc(GL_NOTEQUAL, 0, 0xff);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glDisable(GL_STENCIL_TEST);
    glDisableVertexAttribArray(gl->posloc);
    glDeleteBuffers(1, &quads);

    free(verts);
    free(subs);
}

// Draw the line segment p1-p2 considering the angle of p0-p1 and p2-p3.
static inline unsigned
miter(Pgpt w,
      Pgpt p0,
      Pgpt p1,
      Pgpt p2,
      Pgpt p3,
      Pgpt *out,
      unsigned n)
{
    Pgpt vp = pg_normalize(pg_sub_pts(p1, p0));
    Pgpt vc = pg_normalize(pg_sub_pts(p2, p1));
    Pgpt vn = pg_normalize(pg_sub_pts(p3, p2));
    Pgpt ni = pg_normalize(pg_add_pts(vp, vc));
    Pgpt no = pg_normalize(pg_add_pts(vc, vn));
    float mi = 1.0f / pg_dot(ni, vc);
    float mo = 1.0f / pg_dot(no, vc);
    Pgpt a = pg_sub_pts(p1, pg_mul_pts(perp(ni), pg_scale_pt(w, mi)));
    Pgpt b = pg_add_pts(p1, pg_mul_pts(perp(ni), pg_scale_pt(w, mi)));
    Pgpt c = pg_sub_pts(p2, pg_mul_pts(perp(no), pg_scale_pt(w, mo)));
    Pgpt d = pg_add_pts(p2, pg_mul_pts(perp(no), pg_scale_pt(w, mo)));
    out[n++] = a, out[n++] = b, out[n++] = c;
    out[n++] = b, out[n++] = c, out[n++] = d;
    return n;
}

static inline unsigned
startcap(Pgpt w,
         Pgpt cap,
         Pgpt p1,
         Pgpt p2,
         Pgpt p3,
         Pgpt *out,
         unsigned n)
{
    Pgpt vc = pg_normalize(pg_sub_pts(p2, p1));
    Pgpt vn = pg_normalize(pg_sub_pts(p3, p2));
    Pgpt no = pg_normalize(pg_add_pts(vc, vn));
    float mo = 1.0f / pg_dot(no, vc);
    Pgpt a = pg_sub_pts(pg_sub_pts(p1, pg_mul_pts(perp(vc), w)), pg_mul_pts(vc, cap));
    Pgpt b = pg_sub_pts(pg_add_pts(p1, pg_mul_pts(perp(vc), w)), pg_mul_pts(vc, cap));
    Pgpt c = pg_sub_pts(p2, pg_mul_pts(perp(no), pg_scale_pt(w, mo)));
    Pgpt d = pg_add_pts(p2, pg_mul_pts(perp(no), pg_scale_pt(w, mo)));
    out[n++] = a, out[n++] = b, out[n++] = c;
    out[n++] = b, out[n++] = c, out[n++] = d;
    return n;
}

static inline unsigned
endcap(Pgpt w,
       Pgpt cap,
       Pgpt p0,
       Pgpt p1,
       Pgpt p2,
       Pgpt *out,
       unsigned n)
{
    Pgpt vp = pg_normalize(pg_sub_pts(p1, p0));
    Pgpt vc = pg_normalize(pg_sub_pts(p2, p1));
    Pgpt ni = pg_normalize(pg_add_pts(vp, vc));
    float mi = 1.0f / pg_dot(ni, vc);
    Pgpt a = pg_sub_pts(p1, pg_mul_pts(perp(ni), pg_scale_pt(w, mi)));
    Pgpt b = pg_add_pts(p1, pg_mul_pts(perp(ni), pg_scale_pt(w, mi)));
    Pgpt c = pg_add_pts(pg_sub_pts(p2, pg_mul_pts(perp(vc), w)), pg_mul_pts(vc, cap));
    Pgpt d = pg_add_pts(pg_add_pts(p2, pg_mul_pts(perp(vc), w)), pg_mul_pts(vc, cap));
    out[n++] = a, out[n++] = b, out[n++] = c;
    out[n++] = b, out[n++] = c, out[n++] = d;
    return n;
}

static void _stroke(Pg *g) {
    GL          *gl = pg_get_canvas_impl(g);
    Pgpt        *verts;
    unsigned    *subs;
    unsigned    nverts;
    unsigned    nsubs;

    flatten(g, &verts, &nverts, &subs, &nsubs);

    // Line width is not in device coordinates, so scale it with CTM.
    Pgmat       strokectm = pg_get_ctm(g);
    strokectm.e = 0.0f;
    strokectm.f = 0.0f;
    float       lw = 0.5f * pg_get_line_width(g);
    Pgpt        w = pg_apply_mat(strokectm, pg_pt(lw, lw));

    Pgpt cap = pg_get_line_cap(g) == PG_BUTT_CAP? pg_pt(0.0f, 0.0f):
               pg_get_line_cap(g) == PG_SQUARE_CAP? w:
               pg_pt(0.0f, 0.0f);

    Pgpt        *final = malloc(6 * nverts * sizeof *final);
    unsigned    nfinal = 0;

    for (unsigned s = 0; s < nsubs; s++) {
        bool        closed = subs[s] & CLOSED;
        unsigned    start = SUB(subs[s]);
        unsigned    end = SUB(subs[s + 1]);

        if (closed) {
            end -= 1;
            nfinal = miter(w,
                            verts[end - 1], verts[start],
                            verts[start + 1], verts[start + 2],
                            final, nfinal);
            for (unsigned i = start + 1; i + 2 <= end; i++)
                nfinal = miter(w,
                                verts[i - 1], verts[i],
                                verts[i + 1], verts[i + 2],
                                final, nfinal);
            nfinal = miter(w,
                            verts[end - 2], verts[end - 1],
                            verts[start], verts[start + 1],
                            final, nfinal);
        } else if (end - start <= 2) {
            if (end - start == 2) {
                Pgpt nv = pg_normalize(pg_sub_pts(verts[1], verts[0]));
                Pgpt wv = pg_mul_pts(perp(nv), w);
                Pgpt cv = pg_mul_pts(nv, cap);
                Pgpt a = pg_sub_pts(pg_sub_pts(verts[0], wv), cv);
                Pgpt b = pg_sub_pts(pg_add_pts(verts[0], wv), cv);
                Pgpt c = pg_add_pts(pg_sub_pts(verts[1], wv), cv);
                Pgpt d = pg_add_pts(pg_add_pts(verts[1], wv), cv);
                final[nfinal++] = a, final[nfinal++] = b, final[nfinal++] = c;
                final[nfinal++] = b, final[nfinal++] = c, final[nfinal++] = d;
            }
        } else {
            nfinal = startcap(w, cap,
                                verts[start], verts[start + 1],
                                verts[start + 2], final, nfinal);
            for (unsigned i = start + 1; i + 2 < end; i++)
                nfinal = miter(w,
                                verts[i - 1], verts[i],
                                verts[i + 1], verts[i + 2],
                                final, nfinal);
            nfinal = endcap(w, cap,
                            verts[end - 3], verts[end - 2],
                            verts[end - 1], final, nfinal);
        }
    }

    set_coords(g);
    set_paint(g, pg_get_stroke(g));

    glDisable(GL_STENCIL_TEST);
    GLuint  src = make_buffer(GL_ARRAY_BUFFER, final, nfinal * sizeof *final);
    glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(gl->posloc);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) nfinal);
    glDisableVertexAttribArray(gl->posloc);
    glDeleteVertexArrays(1, &src);

    free(verts);
    free(subs);
    free(final);
}

static void _fill_stroke(Pg *g) {
    _fill(g);
    _stroke(g);
}

static const Pgcanvas_methods methods = {
    .init = _init,
    .free = _free,
    .clear = _clear,
    .fill = _fill,
    .stroke = _stroke,
    .fill_stroke = _fill_stroke,
};

Pg *pg_opengl_canvas(unsigned width, unsigned height) {
    return pg_new_canvas(&methods, width, height);
}
