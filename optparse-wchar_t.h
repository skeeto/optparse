/* Optparse --- portable, reentrant, embeddable, getopt-like option parser
 *
 * This is free and unencumbered software released into the public domain.
 *
 * To get the implementation, define OPTPARSE_IMPLEMENTATION.
 * Optionally define OPTPARSE_API to control the API's visibility
 * and/or linkage (static, __attribute__, __declspec).
 *
 * The POSIX getopt() option parser has three fatal flaws. These flaws
 * are solved by Optparse.
 *
 * 1) Parser state is stored entirely in global variables, some of
 * which are static and inaccessible. This means only one thread can
 * use getopt(). It also means it's not possible to recursively parse
 * nested sub-arguments while in the middle of argument parsing.
 * Optparse fixes this by storing all state on a local struct.
 *
 * 2) The POSIX standard provides no way to properly reset the parser.
 * This means for portable code that getopt() is only good for one
 * run, over one argv with one option string. It also means subcommand
 * options cannot be processed with getopt(). Most implementations
 * provide a method to reset the parser, but it's not portable.
 * Optparse provides an optparse_arg() function for stepping over
 * subcommands and continuing parsing of options with another option
 * string. The Optparse struct itself can be passed around to
 * subcommand handlers for additional subcommand option parsing. A
 * full reset can be achieved by with an additional optparse_init().
 *
 * 3) Error messages are printed to stderr. This can be disabled with
 * opterr, but the messages themselves are still inaccessible.
 * Optparse solves this by writing an error message in its errmsg
 * field. The downside to Optparse is that this error message will
 * always be in English rather than the current locale.
 *
 * Optparse should be familiar with anyone accustomed to getopt(), and
 * it could be a nearly drop-in replacement. The option string is the
 * same and the fields have the same names as the getopt() global
 * variables (optarg, optind, optopt).
 *
 * Optparse also supports GNU-style long options with optparse_long().
 * The interface is slightly different and simpler than getopt_long().
 *
 * By default, argv is permuted as it is parsed, moving non-option
 * arguments to the end. This can be disabled by setting the `permute`
 * field to 0 after initialization.
 */
#ifndef OPTPARSE_H
#define OPTPARSE_H

#ifndef OPTPARSE_API
#  define OPTPARSE_API
#endif

#ifndef OPTPARSE_ARG_TYPE
#  define OPTPARSE_ARG_TYPE char
#endif

struct optparse {
    OPTPARSE_ARG_TYPE **argv;
    int permute;
    int optind;
    int optopt;
    OPTPARSE_ARG_TYPE *optarg;
    OPTPARSE_ARG_TYPE errmsg[64];
    int subopt;
};

enum optparse_argtype {
    OPTPARSE_NONE,
    OPTPARSE_REQUIRED,
    OPTPARSE_OPTIONAL
};

struct optparse_long {
    const OPTPARSE_ARG_TYPE *longname;
    int shortname;
    enum optparse_argtype argtype;
};

/**
 * Initializes the parser state.
 */
OPTPARSE_API
void optparse_init(struct optparse *options, OPTPARSE_ARG_TYPE **argv);

/**
 * Read the next option in the argv array.
 * @param optstring a getopt()-formatted option string.
 * @return the next option character, -1 for done, or '?' for error
 *
 * Just like getopt(), a character followed by no colons means no
 * argument. One colon means the option has a required argument. Two
 * colons means the option takes an optional argument.
 */
OPTPARSE_API
int optparse(struct optparse *options, const OPTPARSE_ARG_TYPE *optstring);

/**
 * Handles GNU-style long options in addition to getopt() options.
 * This works a lot like GNU's getopt_long(). The last option in
 * longopts must be all zeros, marking the end of the array. The
 * longindex argument may be NULL.
 */
OPTPARSE_API
int optparse_long(struct optparse *options,
                  const struct optparse_long *longopts,
                  int *longindex);

/**
 * Used for stepping over non-option arguments.
 * @return the next non-option argument, or NULL for no more arguments
 *
 * Argument parsing can continue with optparse() after using this
 * function. That would be used to parse the options for the
 * subcommand returned by optparse_arg(). This function allows you to
 * ignore the value of optind.
 */
OPTPARSE_API
OPTPARSE_ARG_TYPE *optparse_arg(struct optparse *options);

/* Implementation */
#ifdef OPTPARSE_IMPLEMENTATION

#define OPTPARSE_MSG_INVALID L"invalid option"
#define OPTPARSE_MSG_MISSING L"option requires an argument"
#define OPTPARSE_MSG_TOOMANY L"option takes no arguments"

static int
optparse_error(struct optparse *options, const OPTPARSE_ARG_TYPE *msg, const OPTPARSE_ARG_TYPE *data)
{
    unsigned p = 0;
    const OPTPARSE_ARG_TYPE *sep = L" -- '";
    while (*msg)
        options->errmsg[p++] = *msg++;
    while (*sep)
        options->errmsg[p++] = *sep++;
    while (p < sizeof(options->errmsg) - 2 && *data)
        options->errmsg[p++] = *data++;
    options->errmsg[p++] = L'\'';
    options->errmsg[p++] = L'\0';
    return L'?';
}

OPTPARSE_API
void
optparse_init(struct optparse *options, OPTPARSE_ARG_TYPE **argv)
{
    options->argv = argv;
    options->permute = 1;
    options->optind = 1;
    options->subopt = 0;
    options->optarg = 0;
    options->errmsg[0] = L'\0';
}

static int
optparse_is_dashdash(const OPTPARSE_ARG_TYPE *arg)
{
    return arg != 0 && arg[0] == L'-' && arg[1] == L'-' && arg[2] == L'\0';
}

static int
optparse_is_shortopt(const OPTPARSE_ARG_TYPE *arg)
{
    return arg != 0 && arg[0] == L'-' && arg[1] != L'-' && arg[1] != L'\0';
}

static int
optparse_is_longopt(const OPTPARSE_ARG_TYPE *arg)
{
    return arg != 0 && arg[0] == L'-' && arg[1] == L'-' && arg[2] != L'\0';
}

static void
optparse_permute(struct optparse *options, int index)
{
    OPTPARSE_ARG_TYPE *nonoption = options->argv[index];
    int i;
    for (i = index; i < options->optind - 1; i++)
        options->argv[i] = options->argv[i + 1];
    options->argv[options->optind - 1] = nonoption;
}

static int
optparse_argtype(const OPTPARSE_ARG_TYPE *optstring, OPTPARSE_ARG_TYPE c)
{
    int count = OPTPARSE_NONE;
    if (c == L':')
        return -1;
    for (; *optstring && c != *optstring; optstring++);
    if (!*optstring)
        return -1;
    if (optstring[1] == L':')
        count += optstring[2] == L':' ? 2 : 1;
    return count;
}

OPTPARSE_API
int
optparse(struct optparse *options, const OPTPARSE_ARG_TYPE *optstring)
{
    int type;
    OPTPARSE_ARG_TYPE *next;
    OPTPARSE_ARG_TYPE *option = options->argv[options->optind];
    options->errmsg[0] = L'\0';
    options->optopt = 0;
    options->optarg = 0;
    if (option == 0) {
        return -1;
    } else if (optparse_is_dashdash(option)) {
        options->optind++; /* consume "--" */
        return -1;
    } else if (!optparse_is_shortopt(option)) {
        if (options->permute) {
            int index = options->optind++;
            int r = optparse(options, optstring);
            optparse_permute(options, index);
            options->optind--;
            return r;
        } else {
            return -1;
        }
    }
    option += options->subopt + 1;
    options->optopt = option[0];
    type = optparse_argtype(optstring, option[0]);
    next = options->argv[options->optind + 1];
    switch (type) {
    case -1: {
        OPTPARSE_ARG_TYPE str[2] = {0, 0};
        str[0] = option[0];
        options->optind++;
        return optparse_error(options, OPTPARSE_MSG_INVALID, str);
    }
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
        } else if (next != 0) {
            options->optarg = next;
            options->optind++;
        } else {
            OPTPARSE_ARG_TYPE str[2] = {0, 0};
            str[0] = option[0];
            options->optarg = 0;
            return optparse_error(options, OPTPARSE_MSG_MISSING, str);
        }
        return option[0];
    case OPTPARSE_OPTIONAL:
        options->subopt = 0;
        options->optind++;
        if (option[1])
            options->optarg = option + 1;
        else
            options->optarg = 0;
        return option[0];
    }
    return 0;
}

OPTPARSE_API
OPTPARSE_ARG_TYPE *
optparse_arg(struct optparse *options)
{
    OPTPARSE_ARG_TYPE *option = options->argv[options->optind];
    options->subopt = 0;
    if (option != 0)
        options->optind++;
    return option;
}

static int
optparse_longopts_end(const struct optparse_long *longopts, int i)
{
    return !longopts[i].longname && !longopts[i].shortname;
}

static void
optparse_from_long(const struct optparse_long *longopts, OPTPARSE_ARG_TYPE *optstring)
{
    OPTPARSE_ARG_TYPE *p = optstring;
    int i;
    for (i = 0; !optparse_longopts_end(longopts, i); i++) {
        if (longopts[i].shortname) {
            int a;
            *p++ = (OPTPARSE_ARG_TYPE)longopts[i].shortname;
            for (a = 0; a < (int)longopts[i].argtype; a++)
                *p++ = L':';
        }
    }
    *p = L'\0';
}

/* Unlike strcmp(), handles options containing "=". */
static int
optparse_longopts_match(const OPTPARSE_ARG_TYPE *longname, const OPTPARSE_ARG_TYPE *option)
{
    const OPTPARSE_ARG_TYPE *a = option, *n = longname;
    if (longname == 0)
        return 0;
    for (; *a && *n && *a != L'='; a++, n++)
        if (*a != *n)
            return 0;
    return *n == L'\0' && (*a == L'\0' || *a == L'=');
}

/* Return the part after "=", or NULL. */
static OPTPARSE_ARG_TYPE *
optparse_longopts_arg(OPTPARSE_ARG_TYPE *option)
{
    for (; *option && *option != L'='; option++);
    if (*option == L'=')
        return option + 1;
    else
        return 0;
}

static int
optparse_long_fallback(struct optparse *options,
                       const struct optparse_long *longopts,
                       int *longindex)
{
    int result;
    OPTPARSE_ARG_TYPE optstring[96 * 3 + 1]; /* 96 ASCII printable characters */
    optparse_from_long(longopts, optstring);
    result = optparse(options, optstring);
    if (longindex != 0) {
        *longindex = -1;
        if (result != -1) {
            int i;
            for (i = 0; !optparse_longopts_end(longopts, i); i++)
                if (longopts[i].shortname == options->optopt)
                    *longindex = i;
        }
    }
    return result;
}

OPTPARSE_API
int
optparse_long(struct optparse *options,
              const struct optparse_long *longopts,
              int *longindex)
{
    int i;
    OPTPARSE_ARG_TYPE *option = options->argv[options->optind];
    if (option == 0) {
        return -1;
    } else if (optparse_is_dashdash(option)) {
        options->optind++; /* consume "--" */
        return -1;
    } else if (optparse_is_shortopt(option)) {
        return optparse_long_fallback(options, longopts, longindex);
    } else if (!optparse_is_longopt(option)) {
        if (options->permute) {
            int index = options->optind++;
            int r = optparse_long(options, longopts, longindex);
            optparse_permute(options, index);
            options->optind--;
            return r;
        } else {
            return -1;
        }
    }

    /* Parse as long option. */
    options->errmsg[0] = L'\0';
    options->optopt = 0;
    options->optarg = 0;
    option += 2; /* skip "--" */
    options->optind++;
    for (i = 0; !optparse_longopts_end(longopts, i); i++) {
        const OPTPARSE_ARG_TYPE *name = longopts[i].longname;
        if (optparse_longopts_match(name, option)) {
            OPTPARSE_ARG_TYPE *arg;
            if (longindex)
                *longindex = i;
            options->optopt = longopts[i].shortname;
            arg = optparse_longopts_arg(option);
            if (longopts[i].argtype == OPTPARSE_NONE && arg != 0) {
                return optparse_error(options, OPTPARSE_MSG_TOOMANY, name);
            } if (arg != 0) {
                options->optarg = arg;
            } else if (longopts[i].argtype == OPTPARSE_REQUIRED) {
                options->optarg = options->argv[options->optind];
                if (options->optarg == 0)
                    return optparse_error(options, OPTPARSE_MSG_MISSING, name);
                else
                    options->optind++;
            }
            return options->optopt;
        }
    }
    return optparse_error(options, OPTPARSE_MSG_INVALID, option);
}

#endif /* OPTPARSE_IMPLEMENTATION */
#endif /* OPTPARSE_H */
