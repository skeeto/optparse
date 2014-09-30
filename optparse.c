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

enum argspec { UNDEFINED = 0, NONE, REQUIRED, OPTIONAL };

static enum argspec argspec(const char *spec, char c)
{
    if (c == ':' || c == '?')
        return UNDEFINED;
    for (; *spec && c != *spec; spec++);
    if (!*spec)
        return UNDEFINED;
    enum argspec count = 1;
    if (spec[1] == ':')
        count += spec[2] == ':' ? 2 : 1;
    return count;
}

#define opterror(opts, format, args...)                         \
    snprintf(opts->errmsg, sizeof(opts->errmsg), format, args);

int optparse(struct optparse *opts, const char *spec)
{
    char *arg = opts->argv[opts->optind];
    if (arg == NULL || arg[0] != '-' || arg[1] == '-')
        return -1;
    arg += opts->subopt + 1;
    enum argspec type = argspec(spec, arg[0]);
    char *next = opts->argv[opts->optind + 1];
    switch (type) {
    case UNDEFINED:
        opterror(opts, "invalid option -- '%c'", arg[0]);
        opts->optind++;
        return '?';
    case NONE:
        opts->optarg = NULL;
        if (arg[1]) {
            opts->subopt++;
        } else {
            opts->subopt = 0;
            opts->optind++;
        }
        return arg[0];
    case REQUIRED:
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
    case OPTIONAL:
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
