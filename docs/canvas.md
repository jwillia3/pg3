
Canvas
----------------------------------------------------------------

    struct PgState {
        PgTM                ctm;
        const PgPaint       *fill;
        const PgPaint       *stroke;
        const PgPaint       *clear;
        float               line_width;
        PgLineCap           line_cap;
        float               flatness;
        PgFillRule          fill_rule;
        float               gamma;
        float               clip_x;
        float               clip_y;
        float               clip_sx;
        float               clip_sy;
        PgTextPos           text_pos;
        bool                underline;
    };

    struct Pg {
        const PgCanvasImpl  *v;
        PgPt                size;
        PgPath              *path;
        PgState             s;
        PgState             saved[16];
        unsigned            nsaved;
    };


A canvas must be created in order to draw.


Creating and Disposing
----------------------------------------------------------------

    Pg *pg_opengl(float width, float height);
    void pg_free(Pg *g);

`pg_opengl()` creates a new OpenGL canvas. It does not create a
window or create a GL context. It simply sets up GLEW and the
shader programs that it will need. The client of the library
will need to handle maintenance of the GL context.


Resizing
----------------------------------------------------------------

    void pg_resize(Pg *g, float width, float height);

When a canvas must be resized, it is not enough to change the
structure variables, the underlying implementation must be
notified with `pg_resize()`.


Path Construction
----------------------------------------------------------------

A canvas has a default path associated with it. There are
convenience functions for constructing paths from the canvas
that are analogues of the path construction functions that take
a path parameter.

See [Path](path.md) for details.



State
----------------------------------------------------------------

    bool pg_restore(Pg *g);
    bool pg_save(Pg *g);
    void pg_reset_state(Pg *g);

The state of the canvas can be saved and restored in a stack
with `pg_save()` and `pg_restore()` respectively. Currently, a
maximum of 8 states can be saved at once.

`pg_reset_state()` resets the canvas state. See
[State](state.md) for details.



Transformation
----------------------------------------------------------------

    void pg_identity(Pg *g);
    void pg_translate(Pg *g, float x, float y);
    void pg_rotate(Pg *g, float rads);
    void pg_scale(Pg *g, float x, float y);

There are convenience methods for modifying the current
transform matrix (CTM) that are analogues for the transform
methods. See Transformation Matrix in ([Types](types.md)).



Painting
----------------------------------------------------------------

    void pg_clear(Pg *g);
    void pg_fill(Pg *g);
    void pg_stroke(Pg *g);
    void pg_fill_stroke(Pg *g);

`pg_clear()` applies the clearing paint over the entire clipping
area. `pg_fill()` applies the fill paint over the inside of the
current path and then resets the path. `pg_stroke()` applies the
stroke paint over the outline of the current path and then
resets the path. `pg_fill_stroke()` fills the curent path, then
strokes it, then resets the path. When the stroke paint is
transparent, its is blended with fill paint color where they
intersect.



Implementation
----------------------------------------------------------------

    struct PgCanvasImpl {
        void    (*clear)(Pg *g);
        void    (*fill)(Pg *g);
        void    (*stroke)(Pg *g);
        void    (*fill_stroke)(Pg *g);
        void    (*resize)(Pg *g, float width, float height);
        void    (*free)(Pg *g);
    };

    Pg pg_init_canvas(const PgCanvasImpl *v, float width, float height);

New types of canvases can be implemented by creating a new
structure that contains a `Pg` as its first member and providing
a `PgCanvasImpl` that lists the implementing functions.
