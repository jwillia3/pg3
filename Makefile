SHELL=/bin/sh
.SUFFIXES:
.SUFFIXES: .c .o

prefix=/usr/local
pkgconfigdir=/usr/local/libdata/pkgconfig
libdir=$(prefix)/lib
includedir=$(prefix)/include

CFLAGS=-g -O2
ALL_CFLAGS=-Wall -Wextra -std=c11 -D_XOPEN_SOURCE=700 -Werror=implicit-function-declaration -Iinclude -DUSE_UNIX -DUSE_OPENGL -DUSE_FONTCONFIG -DUSE_XLIB

PKGS=  gl glew fontconfig x11 egl gl glew
PKG_CFLAGS=`pkg-config --cflags $(PKGS)`
PKG_LIBS=`pkg-config --libs $(PKGS)`
LIBS=-lm

SRC!=echo src/*.c
OBJS=$(SRC:.c=.o)

all: libpg3.so .font-viewer .libbox-demo

.font-viewer:
	cd demo/font-viewer && make ../../bin/font-viewer

.libbox-demo:
	cd demo/libbox && make ../../bin/libbox-demo

libpg3.so: $(OBJS)
	$(CC) -fpic -shared -olibpg3.so $(CFLAGS) $(ALL_CFLAGS) $(OBJS) $(PKG_LIBS) $(LIBS)

.c.o: Makefile include/*.h src/*.h
	$(CC) -c -fpic $(ALL_CFLAGS) $(PKG_CFLAGS) $(CFLAGS) $< -o $*.o

clean:
	-rm -rf src/*.o libpg3.so bin/*

install: all
	-mkdir -p $(DESTDIR)$(libdir)
	-mkdir -p $(DESTDIR)$(includedir)/pg
	-mkdir -p $(DESTDIR)$(pkgconfigdir)

	install libpg3.so $(DESTDIR)$(libdir)/
	install include/* $(DESTDIR)$(includedir)/pg/
	install pg3.pc $(DESTDIR)$(pkgconfigdir)/

uninstall:
	-rm -f $(DESTDIR)$(libdir)/libpg3.so
	-rm -rf $(DESTDIR)$(includedir)/include/pg
	-rm -f $(DESTDIR)$(pkgconfigdir)/pg3.pc
