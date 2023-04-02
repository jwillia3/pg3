typedef struct PgCanvasFunc PgCanvasFunc;
typedef struct PgState      PgState;

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
    bool                underline;
    PgState             *next;
};

struct Pg {
    const PgCanvasFunc  *v;
    void                *window;
    void                *user;
    float               sx;
    float               sy;
    PgPath              *path;
    PgState             s;
    PgState             *saved;
};

struct PgCanvasFunc {
    void    (*commit)(Pg *g);
    void    (*clear)(Pg *g);
    void    (*fill)(Pg *g);
    void    (*stroke)(Pg *g);
    void    (*fill_stroke)(Pg *g);
    void    (*resize)(Pg *g, float width, float height);
    void    (*free)(Pg *g);
};

Pg _pg_canvas_init(const PgCanvasFunc *v, float width, float height);
