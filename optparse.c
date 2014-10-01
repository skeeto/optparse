#include <stdio.h>
#include "optparse.h"

void optparse_init(struct optparse *opts, int argc, char **argv)
{
    opts->argc = argc;
    opts->argv = argv;
    opts->optind = 1;
    opts->subopt = 0;
    opts->optarg = NULL;
    opts->errmsg[0] = '\0';
}

static enum optparse_argtype
argtype(const char *optstring, char c)
{
    if (c == ':')
        return -1;
    for (; *optstring && c != *optstring; optstring++);
    if (!*optstring)
        return -1;
    enum optparse_argtype count = OPTPARSE_NONE;
    if (optstring[1] == ':')
        count += optstring[2] == ':' ? 2 : 1;
    return count;
}

#define opterror(opts, format, args...)                         \
    snprintf(opts->errmsg, sizeof(opts->errmsg), format, args);

int optparse(struct optparse *opts, const char *optstring)
{
    opts->errmsg[0] = '\0';
    opts->optopt = 0;
    char *arg = opts->argv[opts->optind];
    if (arg == NULL || arg[0] != '-' || arg[1] == '-')
        return -1;
    arg += opts->subopt + 1;
    opts->optopt = arg[0];
    int type = argtype(optstring, arg[0]);
    char *next = opts->argv[opts->optind + 1];
    switch (type) {
    case -1:
        opterror(opts, "invalid option -- '%c'", arg[0]);
        opts->optind++;
        return '?';
    case OPTPARSE_NONE:
        opts->optarg = NULL;
        if (arg[1]) {
            opts->subopt++;
        } else {
            opts->subopt = 0;
            opts->optind++;
        }
        return arg[0];
    case OPTPARSE_REQUIRED:
        opts->subopt = 0;
        opts->optind++;
        if (arg[1]) {
            opts->optarg = arg + 1;
        } else if (next != NULL) {
            opts->optarg = next;
            opts->optind++;
        } else {
            opterror(opts, "option requires an argument -- '%c'", arg[0]);
            opts->optarg = NULL;
            return '?';
        }
        return arg[0];
    case OPTPARSE_OPTIONAL:
        opts->subopt = 0;
        opts->optind++;
        if (arg[1])
            opts->optarg = arg + 1;
        else
            opts->optarg = NULL;
        return arg[0];
    }
    return 0;
}

char *optparse_arg(struct optparse *opts)
{
    opts->subopt = 0;
    char *arg = opts->argv[opts->optind];
    if (arg != NULL)
        opts->optind++;
    return arg;
}

static inline int
longopts_end(const struct optparse_long *longopts, int i)
{
    return !longopts[i].longname && !longopts[i].shortname;
}

static size_t
optstring_length(const struct optparse_long *longopts)
{
    int length = 0;
    for (; !longopts_end(longopts, length); length++)
        length += longopts[length].argtype;
    return length + 1;
}

static void
optstring_from_long(const struct optparse_long *longopts, char *optstring)
{
    char *p = optstring;
    for (int i = 0; !longopts_end(longopts, i); i++) {
        if (longopts[i].shortname) {
            *p++ = longopts[i].shortname;
            for (int a = 0; a < longopts[i].argtype; a++)
                *p++ = ':';
        }
    }
    *p = '\0';
}

static inline int
is_longopt(const char *arg)
{
    return arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static int
longopts_match(const char *longname, const char *option)
{
    if (longname == NULL)
        return 0;
    const char *a = option, *n = longname;
    for (; *a && *n && *a != '='; a++, n++)
        if (*a != *n)
            return 0;
    return *n == '\0' && (*a == '\0' || *a == '=');
}

static const char *
longopts_arg(const char *option)
{
    for (; *option && *option != '='; option++);
    if (*option == '=')
        return option + 1;
    else
        return NULL;
}

int
optparse_long(struct optparse *options,
              const struct optparse_long *longopts,
              int *longindex)
{
    char *option = options->argv[options->optind];
    if (options->subopt > 0 || !is_longopt(option)) {
        /* Fallback to simple parser. */
        char optstring[optstring_length(longopts)];
        optstring_from_long(longopts, optstring);
        int result = optparse(options, optstring);
        if (longindex != NULL) {
            *longindex = -1;
            if (result != -1)
                for (int i = 0; !longopts_end(longopts, i); i++)
                    if (longopts[i].shortname == options->optopt)
                        *longindex = i;
        }
        return result;
    }
    options->errmsg[0] = '\0';
    options->optopt = 0;
    options->optarg = NULL;
    option += 2;
    options->optind++;
    for (int i = 0; !longopts_end(longopts, i); i++) {
        const char *name = longopts[i].longname;
        if (longopts_match(name, option)) {
            if (longindex)
                *longindex = i;
            options->optopt = longopts[i].shortname;
            const char *arg = longopts_arg(option);
            if (longopts[i].argtype == OPTPARSE_NONE && arg != NULL) {
                opterror(options, "option takes no arguments -- '%s'", name);
                return '?';
            } if (arg != NULL) {
                options->optarg = arg;
            } else if (longopts[i].argtype == OPTPARSE_REQUIRED) {
                options->optarg = options->argv[options->optind++];
                if (options->optarg == NULL) {
                    opterror(options, "option requires argument -- '%s'", name);
                    return '?';
                }
            }
            return options->optopt;
        }
    }
    opterror(options, "invalid option -- '%s'", option);
    return '?';
}
