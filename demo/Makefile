CFLAGS=-Wall -Wextra -I.. -L.. -g
.test: .libpg3
	$(CC) $(CFLAGS) -obox-demo box-demo.c `pkg-config --with-path=.. --cflags --libs libpg3`
	LD_LIBRARY_PATH=..:$(LD_LIBRARY_PATH) ./box-demo

.libpg3:
	cd .. && make
