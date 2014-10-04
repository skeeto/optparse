#include <stdio.h>
#include "optparse.h"

void optparse_init(struct optparse *options, int argc, char **argv)
{
    options->argc = argc;
    options->argv = argv;
    options->optind = 1;
    options->subopt = 0;
    options->optarg = NULL;
    options->errmsg[0] = '\0';
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

#define opterror(options, format, args...)                              \
    snprintf(options->errmsg, sizeof(options->errmsg), format, args);

int optparse(struct optparse *options, const char *optstring)
{
    options->errmsg[0] = '\0';
    options->optopt = 0;
    options->optarg = NULL;
    char *option = options->argv[options->optind];
    if (option == NULL || option[0] != '-') {
        return -1;
    } else if (option[0] == '-' && option[1] == '-' && option[2] == '\0') {
        options->optind++; // consume "--" argument
        return -1;
    } else if (option[1] == '-') {
        return -1;
    }
    option += options->subopt + 1;
    options->optopt = option[0];
    int type = argtype(optstring, option[0]);
    char *next = options->argv[options->optind + 1];
    switch (type) {
    case -1:
        opterror(options, "invalid option -- '%c'", option[0]);
        options->optind++;
        return '?';
    case OPTPARSE_NONE:
        if (option[1]) {
            options->subopt++;
        } else {
            options->subopt = 0;
            options->optind++;
        }
        return option[0];
    case OPTPARSE_REQUIRED:
        options->subopt = 0;
        options->optind++;
        if (option[1]) {
            options->optarg = option + 1;
        } else if (next != NULL) {
            options->optarg = next;
            options->optind++;
        } else {
            opterror(options, "option requires an argument -- '%c'", option[0]);
            options->optarg = NULL;
            return '?';
        }
        return option[0];
    case OPTPARSE_OPTIONAL:
        options->subopt = 0;
        options->optind++;
        if (option[1])
            options->optarg = option + 1;
        else
            options->optarg = NULL;
        return option[0];
    }
    return 0;
}

char *optparse_arg(struct optparse *options)
{
    options->subopt = 0;
    char *option = options->argv[options->optind];
    if (option != NULL)
        options->optind++;
    return option;
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

static int
long_fallback(struct optparse *options,
              const struct optparse_long *longopts,
              int *longindex)
{
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

int
optparse_long(struct optparse *options,
              const struct optparse_long *longopts,
              int *longindex)
{
    char *option = options->argv[options->optind];
    if (option == NULL)
        return -1;
    if (options->subopt > 0 || !is_longopt(option))
        return long_fallback(options, longopts, longindex);
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
