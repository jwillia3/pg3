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

#include <pg-paint.h>
#include <pg-path.h>
#include <pg-font.h>
#include <pg-canvas.h>
#include <pg-window.h>
