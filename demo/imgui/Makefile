.run: ../../bin/demo
	LD_LIBRARY_PATH=../.. ../../bin/demo

../../bin/demo: .libpg3.so demo.c
	$(CC) $(CFLAGS) -g -o../../bin/demo -I../../include -L../.. demo.c -lm -lpg3

.libpg3.so:
	cd ../.. && make libpg3.so
