PLATFORM = linux
USE_OPENGL = 1
USE_GLX = 1

OPTIONS=-DPLATFORM=$(PLATFORM) \
		$(USE_OPENGL:1=-DUSE_OPENGL=1) \
		$(USE_GLX:1=-DUSE_GLX=1)

CFLAGS = -Wall -Wextra  $(PEDANTIC)
ALL_CFLAGS = -std=c11 -D_XOPEN_SOURCE=700 $(OPTIONS) -O2 -g -I. $(CFLAGS)

SRC = pg.c pg.*.c font.*.c canvas.*.c gui.*.c platform.*.c

run-demo: demo
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ./demo


demo:	demo.c libpg3.so
	$(CC) -I. -L. -g -odemo demo.c -lpg3 -lGLEW -lGL -lX11 -lm


libpg3.so: pg3.h internal.h $(SRC)
	$(CC) $(ALL_CFLAGS) -olibpg3.so -fpic -shared $(SRC) -lm -lGLEW -lGL



clean:
	rm libpg3.so demo

install:
	install pg3.h $(DESTDIR)include/
	install libpg3.so $(DESTDIR)lib/

uninstall:
	rm $(DESTDIR)include/pg3.h
	rm $(DESTDIR)lib/libpg3.so
