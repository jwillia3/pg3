typedef struct Pg           Pg;
typedef struct PgTM         PgTM;
typedef enum PgLineCap      PgLineCap;
typedef enum PgFillRule     PgFillRule;

struct PgTM {
    float a;
    float b;
    float c;
    float d;
    float e;
    float f;
};

enum PgLineCap {
    PG_BUTT_CAP,
    PG_SQUARE_CAP,
};

enum PgFillRule {
    PG_NONZERO_RULE,
    PG_EVEN_ODD_RULE,
};


Pg*         pg_canvas_new_opengl(unsigned width, unsigned height);
Pg*         pg_canvas_new_subcanvas(Pg *parent, float x, float y, float sx, float sy);

void        pg_canvas_free(Pg *g);

void        pg_canvas_clear(Pg *g);
void        pg_canvas_fill(Pg *g);
void        pg_canvas_stroke(Pg *g);
void        pg_canvas_fill_stroke(Pg *g);
void        pg_canvas_commit(Pg *g);

float       pg_canvas_printf(Pg *g, PgFont *font, float x, float y, const char *str, ...);
float       pg_canvas_vprintf(Pg *g, PgFont *font, float x, float y, const char *str, va_list ap);
float       pg_canvas_show_char(Pg *g, PgFont *font, float x, float y, uint32_t codepoint);
float       pg_canvas_show_chars(Pg *g, PgFont *font, float x, float y, const char *str, size_t nbytes);
float       pg_canvas_show_string(Pg *g, PgFont *font, float x, float y, const char *str);
float       pg_canvas_show_glyph(Pg *g, PgFont *font, float x, float y, uint32_t glyph);

float       pg_canvas_trace_char(Pg *g, PgFont *font, float x, float y, uint32_t codepoint);
float       pg_canvas_trace_chars(Pg *g, PgFont *font, float x, float y, const char *str, size_t nbytes);
float       pg_canvas_trace_string(Pg *g, PgFont *font, float x, float y, const char *str);
float       pg_canvas_trace_glyph(Pg *g, PgFont *font, float x, float y, uint32_t glyph);

void        pg_canvas_path_clear(Pg *g);
void        pg_canvas_move(Pg *g, float x, float y);
void        pg_canvas_rmove(Pg *g, float x, float y);
void        pg_canvas_line(Pg *g, float x, float y);
void        pg_canvas_rline(Pg *g, float x, float y);
void        pg_canvas_curve3(Pg *g, float bx, float by, float cx, float cy);
void        pg_canvas_rcurve3(Pg *g, float bx, float by, float cx, float cy);
void        pg_canvas_curve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
void        pg_canvas_rcurve4(Pg *g, float bx, float by, float cx, float cy, float dx, float dy);
void        pg_canvas_rectangle(Pg *g, float x, float y, float sx, float sy);
void        pg_canvas_rounded_rectangle(Pg *g, float x, float y, float sx, float sy, float rx, float ry);
void        pg_canvas_close_path(Pg *g);
void        pg_canvas_append_path(Pg *g, const PgPath *src);

PgPt        pg_canvas_get_size(Pg *g);
float       pg_canvas_get_width(Pg *g);
float       pg_canvas_get_height(Pg *g);
void        pg_canvas_set_size(Pg *g, float width, float height);

bool        pg_canvas_state_restore(Pg *g);
bool        pg_canvas_state_save(Pg *g);
void        pg_canvas_state_reset(Pg *g);

void        pg_canvas_identity(Pg *g);
void        pg_canvas_translate(Pg *g, float x, float y);
void        pg_canvas_rotate(Pg *g, float rads);
void        pg_canvas_scale(Pg *g, float x, float y);

void        pg_canvas_set_mat(Pg *g, PgTM tm);
void        pg_canvas_set_fill(Pg *g, const PgPaint *paint);
void        pg_canvas_set_stroke(Pg *g, const PgPaint *paint);
void        pg_canvas_set_clear(Pg *g, const PgPaint *paint);
void        pg_canvas_set_line_width(Pg *g, float line_width);
void        pg_canvas_set_line_cap(Pg *g, PgLineCap line_cap);
void        pg_canvas_set_flatness(Pg *g, float flatness);
void        pg_canvas_set_gamma(Pg *g, float gamma);
void        pg_canvas_set_fill_rule(Pg *g, PgFillRule fill_rule);
void        pg_canvas_set_scissors(Pg *g, float x, float y, float sx, float sy);
void        pg_canvas_set_underline(Pg *g, bool underline);
void        pg_canvas_scissor_reset(Pg *g);

PgTM            pg_canvas_get_mat(Pg *g);
const PgPaint*  pg_canvas_get_fill(Pg *g);
const PgPaint*  pg_canvas_get_stroke(Pg *g);
const PgPaint*  pg_canvas_get_clear(Pg *g);
float           pg_canvas_get_line_width(Pg *g);
PgLineCap       pg_canvas_get_line_cap(Pg *g);
float           pg_canvas_get_flatness(Pg *g);
float           pg_canvas_get_gamma(Pg *g);
PgFillRule      pg_canvas_get_fill_rule(Pg *g);
PgPt            pg_canvas_get_scissor_start(Pg *g);
PgPt            pg_canvas_get_scissor_size(Pg *g);
bool            pg_canvas_get_underline(Pg *g);

PgPt        pg_mat_apply(PgTM ctm, PgPt p);
PgTM        pg_mat_multiply(PgTM x, PgTM y);
PgTM        pg_mat_identity(void);
PgTM        pg_mat_translate(PgTM m, float x, float y);
PgTM        pg_mat_scale(PgTM m, float x, float y);
PgTM        pg_mat_rotate(PgTM m, float rad);
