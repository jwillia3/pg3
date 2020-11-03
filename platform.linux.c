#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <pg3.h>
#include <internal.h>


static char*
xdg_data_home(char tmp[FILENAME_MAX + 1])
{
    if (getenv("XDG_DATA_HOME"))
        return strcpy(tmp, getenv("XDG_DATA_HOME"));

    if (getenv("HOME")) {
        strcpy(tmp, getenv("HOME"));
        return strcat(tmp, "/.local/share");
    }

    return strcat(tmp, "");
}


void*
_pgmap_file(const char *path, size_t *size)
{
    if (!path)
        return 0;

    struct stat st;
    int         fd = open(path, O_RDONLY);

    if (fd < 0)
        return 0;

    if (fstat(fd, &st) < 0)
        return 0;

    *size = (size_t) st.st_size;

    void *data = mmap(0, *size, PROT_READ, MAP_PRIVATE, fd, 0);

    close(fd);

    if (data == MAP_FAILED)
        return 0;

    return data;
}


void
_pgunmap_file(void *ptr, size_t size)
{
    munmap(ptr, size);
}


char **
_pgget_font_files(void)
{
    char        path[FILENAME_MAX * 2 + 1];
    char        *queue[256];
    unsigned    nqueue = 0;
    char        **files = 0;
    unsigned    nfiles = 0;

    // Get roots.
    queue[nqueue++] = strdup("/usr/share/fonts");
    queue[nqueue++] = strdup("/usr/local/share/fonts");
    queue[nqueue++] = strdup(strcat(xdg_data_home(path), "/fonts"));
    if (getenv("HOME")) {
        sprintf(path, "%s/.fonts", getenv("HOME"));
        queue[nqueue++] = strdup(path);
    }

    // Recursively list files in directories.
    while (nqueue) {
        char            *dirname = queue[--nqueue];
        DIR             *dir = opendir(dirname);
        struct dirent   *e;

        while (dir && (e = readdir(dir))) {
            struct stat st;

            sprintf(path, "%s/%s", dirname, e->d_name);

            if (e->d_name[0] == '.') {
                // Ignore hidden files.
            }
            else if (stat(path, &st) >= 0 && S_ISDIR(st.st_mode)) {
                // Queue directories.
                if (nqueue < 256)
                    queue[nqueue++] = strdup(path);
            }
            else {
                // Add files to the list.
                char *ext = strrchr(path, '.');
                static char *extensions[] = {".ttf", ".ttc", ".otf", 0};

                for (char **i = extensions; ext && *i; i++)
                    if (!stricmp(ext, *i)) {
                        files = realloc(files, (nfiles + 2) * sizeof *files);
                        files[nfiles++] = strdup(path);
                        break;
                    }
            }
        }

        free(dirname);
    }

    if (nfiles)
        files[nfiles] = 0;

    return files;
}
