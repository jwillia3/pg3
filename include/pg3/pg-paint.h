typedef struct PgPaint      PgPaint;
typedef struct PgColor      PgColor;
typedef enum PgPaintType    PgPaintType;
typedef enum PgColorSpace   PgColorSpace;

struct PgColor {
    float u;
    float v;
    float w;
    float a;
};

enum PgPaintType {
    PG_SOLID_PAINT,
    PG_LINEAR_PAINT,
};

enum PgColorSpace {
    PG_LINEAR_RGB,
    PG_LCHAB,
    PG_LAB,
    PG_XYZ,
};

struct PgPaint {
    PgPaintType         type;
    PgColorSpace        cspace;
    PgColor             colors[8];
    float               stops[8];
    unsigned            nstops;
    PgPt                a;
    PgPt                b;
    float               ra;
    float               rb;
    bool                immortal;
};


PgColor         pg_color_lch_to_lab(PgColor lch);
PgColor         pg_color_lab_to_xyz(PgColor lab);
PgColor         pg_color_xyz_to_rgb(PgColor xyz);
PgColor         pg_color_gamma_correct(PgColor rgb, float gamma);
PgColor         pg_color_to_rgb(PgColorSpace cspace, PgColor color, float gamma);


const PgPaint   *pg_paint_from_name(const char *name);

PgPaint*        pg_paint_new_linear(PgColorSpace cspace, float ax, float ay, float bx, float by);
PgPaint*        pg_paint_new_solid(PgColorSpace cspace, float u, float v, float w, float a);

void            pg_paint_add_stop(PgPaint *paint, float t, float u, float v, float w, float a);
void            pg_paint_clear_stops(PgPaint *paint);

PgPaint*        pg_paint_clone(const PgPaint *from);
void            pg_paint_free(PgPaint *paint);


PgPaintType     pg_paint_get_type(PgPaint *paint);
PgColorSpace    pg_paint_get_colorspace(PgPaint *paint);
unsigned        pg_paint_get_nstops(PgPaint *paint);
PgColor         pg_paint_get_color(PgPaint *paint, unsigned n);
float           pg_paint_get_stop(PgPaint *paint, unsigned n);
PgPt            pg_paint_get_a(PgPaint *paint);
PgPt            pg_paint_get_b(PgPaint *paint);
float           pg_paint_get_ra(PgPaint *paint);
float           pg_paint_get_rb(PgPaint *paint);
