font-viewer: .libpg3.so font-viewer.c
	$(CC) $(CFLAGS) -I../../include -L../.. -g -Wall -Wextra font-viewer.c -o font-viewer -lm -lpg3

test: font-viewer
	LD_LIBRARY_PATH=../.. ./font-viewer

.libpg3.so:
	make -C../.. libpg3.so
