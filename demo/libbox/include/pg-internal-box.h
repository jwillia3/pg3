struct PgBoxType {
    const char *type;
    PgPt (*measure)(const PgBox *box);
    void (*pack)(PgBox *box);
    void (*paint)(PgBox *box, Pg *g);
    void (*free)(PgBox *box);
    void (*hovered)(PgBox *box, bool is_hovering);
    void (*focused)(PgBox *box, bool is_focused);
    void (*active)(PgBox *box, bool is_active);
};