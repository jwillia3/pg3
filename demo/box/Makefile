.run: ../../bin/demo
	LD_LIBRARY_PATH=../.. ../../bin/demo

../../bin/demo: .libpg3.so *.c
	$(CC) $(CFLAGS) -g -Wall -Wextra -o../../bin/demo -I. -I../../include -L../.. *.c -lm -lpg3

.libpg3.so:
	cd ../.. && make libpg3.so
