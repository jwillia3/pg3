.libbox-demo: ../../bin/libbox-demo
	LD_LIBRARY_PATH=../.. ../../bin/libbox-demo

../../bin/libbox-demo: .libpg3.so demo.c
	$(CC) $(CFLAGS) -o../../bin/libbox-demo -I../../include -Iinclude -L../.. demo.c src/*.c -lm -lpg3

.libpg3.so:
	cd ../.. && make libpg3.so
