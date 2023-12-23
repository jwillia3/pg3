#ifdef USE_FONTCONFIG

#include <ctype.h>
#include <string.h>
#include <fontconfig/fontconfig.h>
#include <pg3/pg.h>
#include <pg3/pg-utf-8.h>

unsigned
_pg_fontconfig_font_dirs(char **queue, unsigned max)
{
    unsigned    n = 0;

    FcStrList *fc_dirs = FcConfigGetFontDirs(0);
    if (!fc_dirs)
        return 0;

    for (char *dir; (dir = (char*) FcStrListNext(fc_dirs)); )
        if (n < max)
            queue[n++] = strdup(dir);
    FcStrListDone(fc_dirs);
    return n;
}


static
bool
exact_family_name_exists(const char *family)
{
    for (const PgFamily *fam = pg_font_list(); fam->name; fam++)
        if (!pg_stricmp(fam->name, family)) return true;
    return false;
}


const char *
_pg_fontconfig_substitute(const char *family)
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
            Make sure this match isn't just a wild guess from
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
            bool is_generic_sans = !pg_stricmp(family, "sans-serif") ||
                                   !pg_stricmp(family, "system-ui") ||
                                   !pg_stricmp(family, "ui-sans-serif");

            is_good_advise = is_generic_sans;

            if (!is_generic_sans && (bogus = FcFontMatch(0, all, &result))) {
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


#else


#include <pg3/pg.h>
#include <pg3/pg-internal-platform.h>


unsigned
_pg_fontconfig_font_dirs(char **queue, unsigned max)
{
    (void) queue, (void) max;
    return 0;
}


const char*
_pg_fontconfig_substitute(const char *family)
{
    (void) family;
    return NULL;
}

#endif
