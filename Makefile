# PEDANTIC=-Weverything -Wno-padded -Wno-cast-qual -Wno-bad-function-cast\
# 	-Wno-format-nonliteral -Wno-undef -Wno-comma -Wno-double-promotion\
# 	-Wno-missing-variable-declarations -Wno-missing-prototypes\
# 	-Wno-disabled-macro-expansion -Wno-shadow -Wno-float-equal\
# 	-Wno-strict-prototypes -pedantic -Wno-switch-enum
CFLAGS=-Wall -Wextra -Werror $(PEDANTIC) -fpic -O2 -g -I. -fno-caret-diagnostics
GUI=opengl
PLATFORM=linux

run-demo: demo
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ./demo

demo:	demo.c libpg.so
	$(CC) $(CFLAGS) -L. -odemo demo.c -lpg -lGLEW -lGL -lglfw -lm

libpg.so: pg.h pg.internal.h pg.c pg.opentype.c pg.$(GUI).c pg.$(PLATFORM).c
	$(CC) $(CFLAGS) -I. -olibpg.so -shared pg.c pg.opentype.c pg.$(GUI).c pg.$(PLATFORM).c  -lm -lGLEW -lGL

clean:
	rm libpg.so demo
