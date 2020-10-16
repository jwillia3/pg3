#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "pg.h"


static char *xdg_data_home(char tmp[FILENAME_MAX + 1]) {
    if (getenv("XDG_DATA_HOME"))
        return strcpy(tmp, getenv("XDG_DATA_HOME"));

    if (getenv("HOME")) {
        strcpy(tmp, getenv("HOME"));
        return strcat(tmp, "/.local/share");
    }

    return strcat(tmp, "");
}

void *_pgmap_file(const char *path, size_t *size) {
    if (!path)
        return 0;

    int fd = open(path, O_RDONLY);
    if (fd < 0)
        return 0;

    struct stat st;
    if (fstat(fd, &st) < 0)
        return 0;
    *size = (size_t) st.st_size;

    void *data = mmap(0, *size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);
    if (data == MAP_FAILED)
        return 0;

    return data;
}

void _pgunmap_file(void *ptr, size_t size) {
    munmap(ptr, size);
}

unsigned _pgget_font_dirs(char *dirs[256]) {
    char        tmp[FILENAME_MAX * 2 + 1];
    unsigned    n = 0;

    dirs[n++] = strdup("/usr/share/fonts");
    dirs[n++] = strdup("/usr/local/share/fonts");

    dirs[n++] = strdup(strcat(xdg_data_home(tmp), "/fonts"));

    if (getenv("HOME")) {
        sprintf(tmp, "%s/.fonts", getenv("HOME"));
        dirs[n++] = strdup(tmp);
    }

    dirs[n] = 0;
    return n;
}
