typedef struct PgPath       PgPath;
typedef enum PgPartType     PgPartType;
typedef struct PgPart       PgPart;

struct PgPath {
    unsigned            nparts;
    PgPart              *parts;
    PgPt                cur;
};

enum PgPartType {
    PG_PART_MOVE,
    PG_PART_LINE,
    PG_PART_CURVE3,
    PG_PART_CURVE4,
    PG_PART_CLOSE,
};

struct PgPart {
    PgPartType          type;
    PgPt                pt[3];
};

PgPath*     pg_path_new(void);
void        pg_path_free(PgPath *path);

void        pg_path_close(PgPath *path);
void        pg_path_curve3(PgPath *path, float bx, float by, float cx, float cy);
void        pg_path_curve4(PgPath *path, float bx, float by, float cx, float cy, float dx, float dy);
void        pg_path_line(PgPath *path, float x, float y);
void        pg_path_rline(PgPath *path, float x, float y);
void        pg_path_move(PgPath *path, float x, float y);
void        pg_path_rmove(PgPath *path, float x, float y);
void        pg_path_rcurve3(PgPath *path, float bx, float by, float cx, float cy);
void        pg_path_rcurve4(PgPath *path, float bx, float by, float cx, float cy, float dx, float dy);
void        pg_path_rectangle(PgPath *path, float x, float y, float sx, float sy);
void        pg_path_rounded(PgPath *path, float x, float y, float sx, float sy, float rx, float ry);
void        pg_path_reset(PgPath *path);
void        pg_path_append(PgPath *path, const PgPath *src);
