PLATFORM=linux
USE_OPENGL=1
USE_GLFW=1

OPTIONS=-DPLATFORM=$(PLATFORM) \
		$(USE_OPENGL:1=-DUSE_OPENGL=1) \
		$(USE_GLFW:1=-DUSE_GLFW=1)

CFLAGS = -Wall -Wextra -fpic -shared -std=c11 -D_XOPEN_SOURCE=700 $(OPTIONS) -O2 -g -I.

libpg3.so: Makefile *.c *.h
	$(CC) \
		$(CFLAGS) \
		`pkg-config --with-path=. --cflags gl glew glfw3 fontconfig` \
		-olibpg3.so \
		*.c \
		`pkg-config --libs gl glew glfw3 fontconfig`

clean:
	rm libpg3.so demo

install:
	install pg3.h pgbox.h $(DESTDIR)include/
	install libpg3.so $(DESTDIR)lib/
	install libpg3.pc `pkg-config --variable pc_path pkg-config | cut -d: -f1`

uninstall:
	rm $(DESTDIR)include/pg3.h
	rm $(DESTDIR)include/pgbox.h
	rm $(DESTDIR)lib/libpg3.so
	rm `pkg-config --variable pc_path pkg-config | cut -d: -f1`/libpg3.pc
