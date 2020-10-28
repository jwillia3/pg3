# PEDANTIC=-Weverything -Wno-padded -Wno-cast-qual -Wno-bad-function-cast\
# 	-Wno-format-nonliteral -Wno-undef -Wno-comma -Wno-double-promotion\
# 	-Wno-missing-variable-declarations -Wno-missing-prototypes\
# 	-Wno-disabled-macro-expansion -Wno-shadow -Wno-float-equal\
# 	-Wno-strict-prototypes -pedantic -Wno-switch-enum
CFLAGS=-Wall -Wextra -Werror $(PEDANTIC) -O2 -g -fno-caret-diagnostics
GUI=opengl
PLATFORM=linux
SRC=pg.c pg.opentype.c pg.$(GUI).c pg.$(PLATFORM).c

run-demo: demo
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ./demo

demo:	demo.c libpg.so
	$(CC) $(CFLAGS) -I. -L. -odemo demo.c -lpg -lGLEW -lGL -lglfw -lm

libpg.so: pg.h pg.internal.h $(SRC)
	$(CC) $(CFLAGS) -I. -olibpg.so -fpic -shared $(SRC) -lm -lGLEW -lGL

clean:
	rm libpg.so demo

install:
	install pg.h $(DESTDIR)include/
	install libpg.so $(DESTDIR)lib/

uninstall:
	rm $(DESTDIR)include/pg.h
	rm $(DESTDIR)lib/libpg.so
