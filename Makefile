PLATFORM=linux
USE_OPENGL=1
USE_GLFW=1

OPTIONS=-DPLATFORM=$(PLATFORM) \
		$(USE_OPENGL:1=-DUSE_OPENGL=1) \
		$(USE_GLFW:1=-DUSE_GLFW=1)

CFLAGS = -Wall -Wextra -fpic -shared -std=c11 -D_XOPEN_SOURCE=700 $(OPTIONS) -O2 -g -I.

SRC = pg.c pg.*.c font.*.c canvas.*.c gui.*.c platform.*.c

# .python: libpg3.so
# 	python3 -Bu demo.py

.demo:	libpg3.so
	$(CC) -I. -L. -odemo demo.c `pkg-config --with-path=. --cflags --libs libpg3`
	LD_LIBRARY_PATH=.:$(LD_LIBRARY_PATH) ./demo

libpg3.so: Makefile $(SRC) *.h
	$(CC) $(CFLAGS) -olibpg3.so `pkg-config --with-path=. --cflags gl glew glfw3 fontconfig` \
		$(SRC) `pkg-config --libs gl glew glfw3 fontconfig`

clean:
	rm libpg3.so demo

install:
	install pg3.h $(DESTDIR)include/
	install libpg3.so $(DESTDIR)lib/
	install libpg3.pc `pkg-config --variable pc_path pkg-config | cut -d: -f1`

uninstall:
	rm $(DESTDIR)include/pg3.h
	rm $(DESTDIR)lib/libpg3.so
	rm `pkg-config --variable pc_path pkg-config | cut -d: -f1`/libpg3.pc
