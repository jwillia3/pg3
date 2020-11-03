GUI = opengl
PLATFORM = linux
SRC = pg.*.c font.*.c gui.$(GUI).c platform.$(PLATFORM).c

# PEDANTIC = -Weverything -Wno-padded -Wno-cast-qual -Wno-bad-function-cast\
# 	-Wno-format-nonliteral -Wno-undef -Wno-comma -Wno-double-promotion\
# 	-Wno-missing-variable-declarations -Wno-missing-prototypes\
# 	-Wno-disabled-macro-expansion -Wno-shadow -Wno-float-equal\
# 	-Wno-strict-prototypes -pedantic -Wno-switch-enum

CFLAGS = -Wall -Wextra -Werror -fno-caret-diagnostics $(PEDANTIC)
ALL_CFLAGS = -std=c11 -D_XOPEN_SOURCE=700 -O2 -g -I. $(CFLAGS)



run-demo: demo
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ./demo


demo:	demo.c libpg.so
	$(CC) $(ALL_CFLAGS) -I. -L. -odemo demo.c -lpg -lGLEW -lGL -lglfw -lm


libpg.so: pg3.h internal.h $(SRC)
	$(CC) $(ALL_CFLAGS) -olibpg.so -fpic -shared $(SRC) -lm -lGLEW -lGL



clean:
	rm libpg.so demo

install:
	install {pg3.h,pg3.util.h} $(DESTDIR)include/
	install libpg.so $(DESTDIR)lib/

uninstall:
	rm $(DESTDIR)include/{pg3.h,pg3.util.h}
	rm $(DESTDIR)lib/libpg.so
