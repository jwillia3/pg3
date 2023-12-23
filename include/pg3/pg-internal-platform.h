void*       _pg_file_map(const char *path, size_t *sizep);
void        _pg_file_unmap(void *ptr, size_t size);

unsigned    _pg_default_font_dirs(char **queue, unsigned max);

unsigned    _pg_fontconfig_font_dirs(char **queue, unsigned max);
const char* _pg_fontconfig_substitute(const char *family);

PgPt        _pg_window_get_dpi_platform(PgWindow *win);
