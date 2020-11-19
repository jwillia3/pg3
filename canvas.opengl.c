#if USE_OPENGL == 1

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <GL/glew.h>
#include <pg3.h>
#include <internal.h>

#define BEZIER_LIMIT    10
#define CLOSED          65536
#define SUB(N)          ((N) & (CLOSED - 1))
#define GL(G)           ((GL*) (G))

typedef struct {
    Pg          _;
    GLuint      prog, vsh, fsh;
    GLint       posloc, ctmloc;
} GL;

static const PgCanvasImpl methods;

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
    "uniform vec4    colors[8];",
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
    "vec4 convert(vec4 color) {",
    "    return  cspace == 0? gamma(color):",
    "            cspace == 1? gamma(xyztorgb(labtoxyz(lchtolab(color)))):",
    "            cspace == 2? gamma(xyztorgb(labtoxyz(color))):",
    "            cspace == 3? gamma(xyztorgb(color)):",
    "            gamma(color);",
    "}",
    "vec4 stopcolor(float t) {",
    "    int i;",
    "    for (i = 0; i < nstops && t > stops[i]; i++) {}",
    "    vec4 color =   i == 0? colors[0]:",
    "                    i == nstops? colors[nstops - 1]:",
    "    mix(colors[i - 1],",
    "        colors[i],",
    "        (t - stops[i - 1]) / (stops[i] - stops[i - 1]));",
    "    return convert(color);",
    "}",
    "void main() {",
    "    if (nstops == 1)",
    "        gl_FragColor = convert(colors[0]);",
    "    else if (type == 2) // Radial gradient.",
    "        gl_FragColor = stopcolor(length(vec2(gl_FragCoord) - a) / (rb - ra));",
    "    else { // Linear gradient.",
    "        vec2 dv = b - a;",
    "        vec2 dp = vec2(gl_FragCoord) - a;",
    "        float t = dot(dv, dp) / dot(dv, dv);",
    "        gl_FragColor = stopcolor(t);",
    "    }",
    "}",
    0
};


static PgColor
lch_to_lab(PgColor lch)
{
    float c = lch.y;
    float h = lch.z * 2.0f * 3.14159f;
    return (PgColor) {lch.x, c * cosf(h), c * sinf(h), lch.a};
}


static PgColor
lab_to_xyz(PgColor lab)
{
    float l = lab.x;
    float y = (l + 16.0f) / 116.0f;
    float x = y + lab.y / 500.0f;
    float z = y - lab.z / 200.0f;
    x = x > 24.0f / 116.0f? x*x*x: (x - 16.0f/116.0f) * 108.0f/841.0f;
    y = y > 24.0f / 116.0f? y*y*y: (y - 16.0f/116.0f) * 108.0f/841.0f;
    z = z > 24.0f / 116.0f? z*z*z: (z - 16.0f/116.0f) * 108.0f/841.0f;
    return (PgColor) {x * 950.4f, y * 1000.0f, z * 1088.8f, lab.a};
}


static PgColor
xyz_to_rgb(PgColor xyz)
{
    float x = xyz.x;
    float y = xyz.y;
    float z = xyz.z;
    float r = x * +3.2404542f + y * -1.5371385f + z * -0.4985314f;
    float g = x * -0.9692660f + y * +1.8760108f + z * +0.0415560f;
    float b = x * +0.0556434f + y * -0.2040259f + z * +1.0572252f;
    return (PgColor) {r, g, b, xyz.a};
}


static PgColor
gamma_correct(PgColor rgb)
{
    float r = rgb.x;
    float g = rgb.y;
    float b = rgb.z;
    return (PgColor) {
        r < 0.0031308f? 12.92f * r: 1.055f * powf(r, 1.0f / 2.20f) - 0.055f,
        g < 0.0031308f? 12.92f * g: 1.055f * powf(g, 1.0f / 2.20f) - 0.055f,
        b < 0.0031308f? 12.92f * b: 1.055f * powf(b, 1.0f / 2.20f) - 0.055f,
        rgb.a
    };
}


static PgColor
convert_color(PgColorSpace cspace, PgColor color)
{
    return  cspace == 0? gamma_correct(color):
            cspace == 1? gamma_correct(xyz_to_rgb(lab_to_xyz(lch_to_lab(color)))):
            cspace == 2? gamma_correct(xyz_to_rgb(lab_to_xyz(color))):
            cspace == 3? gamma_correct(xyz_to_rgb(color)):
            color;
}



static inline PgPt
perp(PgPt p)
{
    return PgPt(-p.y, p.x);
}

static inline PgPt
normalize(PgPt p)
{
    float invmag = 1.0f / sqrtf(p.x * p.x + p.y * p.y);
    return PgPt(p.x * invmag, p.y * invmag);
}

static inline float
dot(PgPt a, PgPt b)
{
    return a.x * b.x + a.y * b.y;
}



static GLuint
make_buffer(GLenum target, const void *data, size_t size)
{
    GLuint buf;
    glGenBuffers(1, &buf);
    glBindBuffer(target, buf);
    glBufferData(target, (GLsizei) size, data, GL_STREAM_DRAW);
    return buf;
}


static GLuint
make_shader(GLenum type, const GLchar **srcarray)
{
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


static GLuint
make_program(GLuint vshader, GLuint fshader)
{
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


static void
set_coords(Pg *g)
{
    GL *gl = GL(g);

    glUseProgram(gl->prog);

    glEnable(GL_SCISSOR_TEST);
    glScissor((GLint) g->s.clip_x,
              (GLint) (g->sy - g->s.clip_y - g->s.clip_sy),
              (GLsizei) g->s.clip_sx,
              (GLsizei) g->s.clip_sy);

    float ctm[] = { 2.0f / g->sx, 0.0f, 0.0f,
                    0.0f, -2.0f / g->sy, 0.0f,
                    -1.0f, 1.0f, 0.0f };
    glUniformMatrix3fv(gl->ctmloc, 1, false, ctm);
}


static void
set_paint(Pg *g, const PgPaint *paint)
{
    GL      *gl = GL(g);
    GLuint  prog = gl->prog;

    glUniform1i(glGetUniformLocation(prog, "nstops"), paint->nstops);

    for (unsigned i = 0; i < paint->nstops; i++) {
        char tmp[16];
        snprintf(tmp, sizeof tmp, "colors[%d]", i);
        glUniform4f(glGetUniformLocation(prog, tmp),
            paint->colors[i].x,
            paint->colors[i].y,
            paint->colors[i].z,
            paint->colors[i].a);

        snprintf(tmp, sizeof tmp, "stops[%d]", i);
        glUniform1f(glGetUniformLocation(prog, tmp), paint->stops[i]);
    }

    float h = g->sy;
    glUniform1i(glGetUniformLocation(prog, "type"), paint->type);
    glUniform1i(glGetUniformLocation(prog, "cspace"), paint->cspace);
    glUniform2f(glGetUniformLocation(prog, "a"), paint->a.x, h - paint->a.y);
    glUniform2f(glGetUniformLocation(prog, "b"), paint->b.x, h - paint->b.y);
    glUniform1f(glGetUniformLocation(prog, "ra"), paint->ra);
    glUniform1f(glGetUniformLocation(prog, "rb"), paint->rb);
}


static void
_free(Pg *g)
{
    GL *gl = GL(g);
    glDeleteShader(gl->vsh);
    glDeleteShader(gl->fsh);
    glDeleteProgram(gl->prog);
    free(gl);
}


static void
_clear(Pg *g)
{
    const PgPaint *paint = g->s.clear;

    if (paint->nstops == 1) {
        set_coords(g);

        PgColor c = convert_color(paint->cspace, paint->colors[0]);
        glClearColor(c.x, c.y, c.z, c.a);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    }

    else {
        GL      *gl = GL(g);

        set_coords(g);
        set_paint(g, paint);

        GLfloat verts[] = { 0.0f, 0.0f,
                            g->sx, 0.0f,
                            0.0f, g->sy,
                            g->sx, g->sy };

        GLuint quads = make_buffer(GL_ARRAY_BUFFER, verts, 8 * sizeof *verts);
        glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
        glEnableVertexAttribArray(gl->posloc);

        glDisable(GL_STENCIL_TEST);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

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
        PgPt **pverts,
        unsigned *pnverts,
        unsigned **psubs,
        unsigned *pnsubs)
{
    PgPt        *verts = malloc(65536 * sizeof *verts);
    unsigned    *subs = malloc(65536 * sizeof *subs);
    unsigned    nverts = 0;
    unsigned    nsubs = 0;
    PgTM        ctm = g->s.ctm;
    PgPt        home = pg_apply_tm(ctm, PgPt(0.0f, 0.0f));
    PgPt        cur = home;
    PgPath      path = *g->path;
    float       flatness = (g->s.flatness * 0.5f) * (g->s.flatness * 0.5f);

    for (unsigned i = 0; i < path.nparts; i++) {
        if (nverts >= 65536 - (1 << BEZIER_LIMIT))
            break;

        PgPt    *pts = path.parts[i].pt;
        switch (path.parts[i].type) {
        case PG_PART_MOVE:
            cur = home = verts[nverts++] = pg_apply_tm(ctm, pts[0]);
            subs[nsubs++] = nverts - 1;
            break;
        case PG_PART_LINE:
            cur = verts[nverts++] = pg_apply_tm(ctm, pts[0]);
            break;
        case PG_PART_CURVE3:
            nverts += flatten3(verts + nverts,
                            cur,
                            pg_apply_tm(ctm, pts[0]),
                            pg_apply_tm(ctm, pts[1]),
                            flatness,
                            BEZIER_LIMIT);
            cur = pg_apply_tm(ctm, pts[1]);
            break;
        case PG_PART_CURVE4:
            nverts += flatten4(verts + nverts,
                            cur,
                            pg_apply_tm(ctm, pts[0]),
                            pg_apply_tm(ctm, pts[1]),
                            pg_apply_tm(ctm, pts[2]),
                            flatness,
                            BEZIER_LIMIT);
            cur = pg_apply_tm(ctm, pts[2]);
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


static void
_fill(Pg *g)
{
    GL          *gl = GL(g);
    PgPt        *verts;
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
    set_paint(g, g->s.fill);

    /*
        Draw shape to stencil buffer by fanning triangles from a single vertex.
        For even-odd fill mode, all the triangles flip the bits under them.
        For non-zero winding mode, triangles going counter-clockwise increment
        the stencil, and clockwise decrement.
        For each, non-zero stencil values are drawn.
    */

    GLuint src = make_buffer(GL_ARRAY_BUFFER, verts, nverts * sizeof *verts);
    glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(gl->posloc);

    if (g->s.fill_rule == PG_EVEN_ODD_RULE) {

        glColorMask(0, 0, 0, 0);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0);
        glStencilOp(GL_INVERT, GL_INVERT, GL_INVERT);

        for (unsigned i = 0; i < nsubs; i++) {
            int     start = SUB(subs[i]);
            int     n = SUB(subs[i + 1]) - SUB(subs[i]);
            glDrawArrays(GL_TRIANGLE_FAN, start, n);
        }

        glColorMask(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else {

        glColorMask(0, 0, 0, 0);
        glEnable(GL_CULL_FACE);
        glEnable(GL_STENCIL_TEST);
        glStencilFunc(GL_ALWAYS, 0, 0);

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
        glColorMask(1.0f, 1.0f, 1.0f, 1.0f);
    }

    glDisableVertexAttribArray(gl->posloc);
    glDeleteBuffers(1, &src);

    // Draw a quad over mask only placing pixels where the stencil bit is set.

    PgPt min = verts[0];
    PgPt max = verts[0];

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

    glDisableVertexAttribArray(gl->posloc);
    glDeleteBuffers(1, &quads);

    free(verts);
    free(subs);
}


// Draw the line segment p1-p2 considering the angle of p0-p1 and p2-p3.
static inline unsigned
miter(PgPt w,
      PgPt p0,
      PgPt p1,
      PgPt p2,
      PgPt p3,
      PgPt *out,
      unsigned n)
{
    PgPt vp = normalize(sub(p1, p0));
    PgPt vc = normalize(sub(p2, p1));
    PgPt vn = normalize(sub(p3, p2));
    PgPt ni = normalize(add(vp, vc));
    PgPt no = normalize(add(vc, vn));
    float mi = 1.0f / dot(ni, vc);
    float mo = 1.0f / dot(no, vc);
    PgPt a = sub(p1, mul(perp(ni), scale_pt(w, mi)));
    PgPt b = add(p1, mul(perp(ni), scale_pt(w, mi)));
    PgPt c = sub(p2, mul(perp(no), scale_pt(w, mo)));
    PgPt d = add(p2, mul(perp(no), scale_pt(w, mo)));
    out[n++] = a, out[n++] = b, out[n++] = c;
    out[n++] = b, out[n++] = c, out[n++] = d;
    return n;
}


static inline unsigned
startcap(PgPt w,
         PgPt cap,
         PgPt p1,
         PgPt p2,
         PgPt p3,
         PgPt *out,
         unsigned n)
{
    PgPt vc = normalize(sub(p2, p1));
    PgPt vn = normalize(sub(p3, p2));
    PgPt no = normalize(add(vc, vn));
    float mo = 1.0f / dot(no, vc);
    PgPt a = sub(sub(p1, mul(perp(vc), w)), mul(vc, cap));
    PgPt b = sub(add(p1, mul(perp(vc), w)), mul(vc, cap));
    PgPt c = sub(p2, mul(perp(no), scale_pt(w, mo)));
    PgPt d = add(p2, mul(perp(no), scale_pt(w, mo)));
    out[n++] = a, out[n++] = b, out[n++] = c;
    out[n++] = b, out[n++] = c, out[n++] = d;
    return n;
}


static inline unsigned
endcap(PgPt w,
       PgPt cap,
       PgPt p0,
       PgPt p1,
       PgPt p2,
       PgPt *out,
       unsigned n)
{
    PgPt vp = normalize(sub(p1, p0));
    PgPt vc = normalize(sub(p2, p1));
    PgPt ni = normalize(add(vp, vc));
    float mi = 1.0f / dot(ni, vc);
    PgPt a = sub(p1, mul(perp(ni), scale_pt(w, mi)));
    PgPt b = add(p1, mul(perp(ni), scale_pt(w, mi)));
    PgPt c = add(sub(p2, mul(perp(vc), w)), mul(vc, cap));
    PgPt d = add(add(p2, mul(perp(vc), w)), mul(vc, cap));
    out[n++] = a, out[n++] = b, out[n++] = c;
    out[n++] = b, out[n++] = c, out[n++] = d;
    return n;
}


static void
_stroke(Pg *g)
{
    GL          *gl = GL(g);
    PgPt        *verts;
    unsigned    *subs;
    unsigned    nverts;
    unsigned    nsubs;

    flatten(g, &verts, &nverts, &subs, &nsubs);

    // Line width is not in device coordinates, so scale it with CTM.
    PgTM    strokectm = { g->s.ctm.a, g->s.ctm.b, g->s.ctm.c, g->s.ctm.d, 0.0f, 0.0f };
    float   lw = 0.5f * g->s.line_width;
    PgPt    w = pg_apply_tm(strokectm, PgPt(lw, lw));

    PgPt    cap = g->s.line_cap == PG_BUTT_CAP? PgPt(0.0f, 0.0f):
                  g->s.line_cap == PG_SQUARE_CAP? w:
                  PgPt(0.0f, 0.0f);


    // Construct each subpath.

    PgPt        *final = malloc(6 * nverts * sizeof *final);
    unsigned    nfinal = 0;

    for (unsigned s = 0; s < nsubs; s++) {
        bool        closed = subs[s] & CLOSED;
        unsigned    start = SUB(subs[s]);
        unsigned    end = SUB(subs[s + 1]);

        if (closed) {
            // There are no caps on this line.
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
        }

        else if (end - start <= 2) {

            // There is only one segment.

            if (end - start == 2) {
                PgPt nv = normalize(sub(verts[1], verts[0]));
                PgPt wv = mul(perp(nv), w);
                PgPt cv = mul(nv, cap);
                PgPt a = sub(sub(verts[0], wv), cv);
                PgPt b = sub(add(verts[0], wv), cv);
                PgPt c = add(sub(verts[1], wv), cv);
                PgPt d = add(add(verts[1], wv), cv);
                final[nfinal++] = a, final[nfinal++] = b, final[nfinal++] = c;
                final[nfinal++] = b, final[nfinal++] = c, final[nfinal++] = d;
            }
        }

        else {

            // There are multiple segments.

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
    set_paint(g, g->s.stroke);

    GLuint  src = make_buffer(GL_ARRAY_BUFFER, final, nfinal * sizeof *final);
    glVertexAttribPointer(gl->posloc, 2, GL_FLOAT, 0, 0, 0);
    glEnableVertexAttribArray(gl->posloc);

    glDisable(GL_STENCIL_TEST);
    glDrawArrays(GL_TRIANGLES, 0, (GLsizei) nfinal);

    glDisableVertexAttribArray(gl->posloc);
    glDeleteVertexArrays(1, &src);

    free(verts);
    free(subs);
    free(final);
}


static void
_fill_stroke(Pg *g)
{
    _fill(g);
    _stroke(g);
}


static void
_resize(Pg *g, float width, float height)
{
    (void) g;
    glViewport(0.0f, 0.0f, (GLsizei) width, (GLsizei) height);
}


Pg*
pg_opengl(float width, float height)
{
    glewInit();

    GLuint  vsh = make_shader(GL_VERTEX_SHADER, VERTEX_SHADER);
    GLuint  fsh = make_shader(GL_FRAGMENT_SHADER, FRAGMENT_SHADER);
    GLuint  prog = make_program(vsh, fsh);
    GLint   posloc = glGetAttribLocation(prog, "pos");
    GLint   ctmloc = glGetUniformLocation(prog, "ctm");

    return new(GL,
                pg_init_canvas(&methods, width, height),
                prog,
                vsh,
                fsh,
                posloc,
                ctmloc);
}

static const PgCanvasImpl methods = {
    _clear,
    _fill,
    _stroke,
    _fill_stroke,
    _resize,
    _free,
};

#endif
