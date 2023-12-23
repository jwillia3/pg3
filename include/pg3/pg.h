#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define pgnew(t, ...) memcpy(malloc(sizeof(t)), &(t){__VA_ARGS__}, sizeof(t))
#define pgclone(t) memcpy(malloc(sizeof(t)), &t, sizeof(t))
#define pgcopy(n, t) memcpy(malloc((n)*sizeof(*t)), t, (n)*sizeof(*t))

typedef struct PgPt         PgPt;

struct PgPt {
    float x;
    float y;
};

#define pgpt(X, Y)              ((PgPt) { (X), (Y) })

#include <pg3/pg-paint.h>
#include <pg3/pg-path.h>
#include <pg3/pg-font.h>
#include <pg3/pg-canvas.h>
#include <pg3/pg-window.h>
