General Types
----------------------------------------------------------------

    struct PgColor {
        float x;
        float y;
        float z;
        float a;
    };

    struct PgPt {
        float x;
        float y;
    };

    struct PgTM {
        float a;
        float b;
        float c;
        float d;
        float e;
        float f;
    };


Transformation Matrix
----------------------------------------------------------------

    PgPt pg_apply_tm(PgTM ctm, PgPt p);
    PgTM pg_mul_tm(PgTM x, PgTM y);
    PgTM pg_ident_tm(void);
    PgTM pg_translate_tm(PgTM m, float x, float y);
    PgTM pg_scale_tm(PgTM m, float x, float y);
    PgTM pg_rotate_tm(PgTM m, float rad);


Points
----------------------------------------------------------------

    #define PgPt(X, Y)
