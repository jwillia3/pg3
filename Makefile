# PEDANTIC=-Weverything -Wno-padded -Wno-cast-qual -Wno-bad-function-cast\
# 	-Wno-format-nonliteral -Wno-undef -Wno-comma -Wno-double-promotion\
# 	-Wno-missing-variable-declarations -Wno-missing-prototypes\
# 	-Wno-disabled-macro-expansion -Wno-shadow -Wno-float-equal\
# 	-Wno-strict-prototypes -pedantic
CFLAGS=-Wall -Wextra -Werror $(PEDANTIC) -O2 -g -I. -fno-caret-diagnostics
GUI=opengl
PLATFORM=linux

run-demo: demo
	./demo

demo:	demo.c libpg.a
	$(CC) $(CFLAGS) -L. -odemo demo.c -lpg -lGLEW -lGL -lglfw -lm

libpg.a: pg.c pg.h pgutil.h pg.opentype.c pg.$(GUI).c pg.$(PLATFORM).c
	$(CC) $(CFLAGS) -c pg.c pg.opentype.c pg.$(GUI).c pg.$(PLATFORM).c
	ar crs libpg.a pg.o pg.opentype.o pg.$(GUI).o pg.$(PLATFORM).o

clean:
	rm *.o *.a demo
