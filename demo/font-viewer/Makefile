.font-viewer: ../../bin/font-viewer
	LD_LIBRARY_PATH=../.. ../../bin/font-viewer

../../bin/font-viewer: .libpg3.so font-viewer.c
	$(CC) $(CFLAGS) -o../../bin/font-viewer -I../../include -L../.. font-viewer.c -lm -lpg3

.libpg3.so:
	cd ../.. && make libpg3.so
