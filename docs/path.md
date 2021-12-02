Path
----------------------------------------------------------------

In order to draw to a canvas, you must construct a path and then
fill it to color the inside of the path or stroke it to draw
lines around the edge of the path.

Paths are constructed by moving a pen around. Paths begin with a
`pg_move()` to set the starting position. The path is traced
using `pg_line()`, `pg_curve3()`, and `pg_curve4` to trace
lines, quadratic, and cubic Bezier curves, respectively.

To draw a closed shape, the path should be closed with
`pg_close()` which will draw a straight line to position of the
last `pg_move()`. If the path is not closed, `pg_fill()` acts
the same. `pg_stroke()` however, will not connect the last point
to the first.

If you intend a shape to be closed, you *must* call
`pg_close()`. Otherwise, the last point will not be properly
connected to the first *even if* they are at the same
coordinates.

Moving the pen before filling or stroking causes a new _subpath_
to be created. After this, `pg_close()` will connect to the last
place the pen was moved. When moving the pen, the previous path
is not closed automatically.


Filling and Stroking
----------------------------------------------------------------

Once the path has been traced, it can be drawn to the canvas by
calling `pg_fill()` to paint the inside of the path,
`pg_stroke()` to draw lines around the edge of the path, or
`pg_fill_stroke()` to do both. A few parts of the canvas state
effect how the path is drawn. See [State](state.md) for details.


Traversal
----------------------------------------------------------------

With a path object, you can traverse its parts which represent
each call such as `pg_move()`, `pg_line`, etc.

    PgPath path = pg_get_path(g);
    for (unsigned i = 0; i < path.nparts; i++) {
        PgPt *p = path.parts[i].pt;
        switch (path.parts[i].type) {
        case PG_PART_MOVE:
            printf("pg_move(g, %g, %g);\n", p[0].x, p[0].y);
            break;
        case PG_PART_LINE:
            printf("pg_line(g, %g, %g);\n", p[0].x, p[0].y);
            break;
        case PG_PART_CURVE3:
            printf("pg_curve3(g, %g, %g, %g, %g);\n",
                p[0].x, p[0].y, p[1].x, p[1].y);
            break;
        case PG_PART_CURVE4:
            printf("pg_curve4(g, %g, %g, %g, %g, %g, %g);\n",
                p[0].x, p[0].y, p[1].x, p[1].y, p[2].x, p[2].y);
            break;
        case PG_PART_CLOSE:
            printf("pg_close(g);\n");
            break;
        }
    }
