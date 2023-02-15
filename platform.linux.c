#if PLATFORM==linux


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

#include <fontconfig/fontconfig.h>

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


static
bool
exact_family_name_exists(const char *family)
{
    for (PgFamily *fam = pg_list_fonts(); fam->name; fam++)
        if (!stricmp(fam->name, family)) return true;
    return false;
}

const char *
_pg_advise_family_replace(const char *family)
{
    FcChar8     *new_family = 0;
    FcResult    result;
    FcPattern   *match;
    FcPattern   *pat = FcPatternBuild(0,
                                      FC_FAMILY, FcTypeString, family,
                                      FC_OUTLINE, FcTypeBool, true,
                                      NULL);
    if (!pat)
        return 0;
    FcConfigSubstitute(0, pat, FcMatchPattern);
    FcDefaultSubstitute(pat);
    if ((match = FcFontMatch(0, pat, &result))) {
        /*
            Make sure we this match isn't just a guess from
            fontconfig. fontconfig will never return no matches.
            If the first family name that comes up is the same
            as searching with no criteria, the match is baseless
            and we shouldn't use it.
        */
        FcChar8     *bogus_family = 0;
        FcPattern   *all = FcPatternCreate();
        FcPattern   *bogus;
        bool        is_good_advise = false;
        if (all) {
            FcConfigSubstitute(0, all, FcMatchPattern);
            FcDefaultSubstitute(all);
            if ((bogus = FcFontMatch(0, all, &result))) {
                FcPatternGetString(match, FC_FAMILY, 0, &new_family);
                FcPatternGetString(bogus, FC_FAMILY, 0, &bogus_family);
                is_good_advise = strcmp((char*) new_family, (char*) bogus_family);
                new_family = 0;
                FcPatternDestroy(bogus);
            }
            FcPatternDestroy(all);
        }

        /*
            Go through list and make sure that fontconfig's alias
            exists in our list. It could recommend a font in a
            format we do not support.
            This loop will result in new_family being set if it was found.
         */
        if (is_good_advise) {
            int alias = 0;
            while (!FcPatternGetString(match, FC_FAMILY, alias, &new_family) &&
                   !exact_family_name_exists((char*) new_family))
            {
                alias++;
                new_family = 0;
            }
        }

        if (new_family) new_family = (void*) strdup((char*) new_family);

        FcPatternDestroy(match);
    }
    FcPatternDestroy(pat);
    return (char*) new_family;
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
    FcStrList *fc_dirs = FcConfigGetFontDirs(0);
    if (fc_dirs) {
        for (char *dir; (dir = (char*) FcStrListNext(fc_dirs)); )
            if (nqueue < 256)
                queue[nqueue++] = strdup(dir);
        FcStrListDone(fc_dirs);
    }
    else {
        queue[nqueue++] = strdup("/usr/share/fonts");
        queue[nqueue++] = strdup("/usr/local/share/fonts");
        queue[nqueue++] = strdup(strcat(xdg_data_home(path), "/fonts"));
        if (getenv("HOME")) {
            sprintf(path, "%s/.fonts", getenv("HOME"));
            queue[nqueue++] = strdup(path);
        }
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
                /*
                    Queue directories.
                    We can do a cursory search of the remaining queue
                    to prevent duplication, but we don't keep track of
                    duplicates we've already processed and removed from
                    the queue.
                 */
                if (nqueue < 256) {
                    char    **dup = queue;
                    char    **end = dup + nqueue;
                    while (dup < end && strcmp(*dup, path)) dup++;
                    if (dup == end)
                        queue[nqueue++] = strdup(path);
                }
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

#endif
