#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void
print_argv(char **argv)
{
    while (*argv) {
        printf("%s ", *argv++);
    }
    printf("\n");
}

static void
try_optparse(char **argv)
{
    int opt;
    char *arg;
    struct optparse options;

    print_argv(argv);
    optparse_init(&options, argv);
    while ((opt = optparse(&options, "abc:d::")) != -1) {
        if (opt == '?') {
            printf("%s: %s\n", argv[0], options.errmsg);
        }
        printf("%c (%d) = '%s'\n", opt, options.optind, options.optarg);
    }
    printf("optind = %d\n", options.optind);
    while ((arg = optparse_arg(&options))) {
        printf("argument: %s\n", arg);
    }
}

static void
try_optparse_long(char **argv)
{
    char *arg;
    int opt, longindex;
    struct optparse options;
    struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_REQUIRED},
        {"delay", 'd', OPTPARSE_OPTIONAL},
        {"erase", 256, OPTPARSE_REQUIRED},
        {0, 0, 0}
    };

    print_argv(argv);
    optparse_init(&options, argv);
    while ((opt = optparse_long(&options, longopts, &longindex)) != -1) {
        char buf[2] = {0, 0};
        if (opt == '?') {
            printf("%s: %s\n", argv[0], options.errmsg);
        }
        buf[0] = opt;
        printf("%-6s(%d, %d) = '%s'\n",
               opt < 127 ? buf : longopts[longindex].longname,
               options.optind, longindex, options.optarg);
    }
    printf("optind = %d\n", options.optind);
    while ((arg = optparse_arg(&options))) {
        printf("argument: %s\n", arg);
    }
}

static int
manual_test(int argc, char **argv)
{
    size_t size = (argc + 1) * sizeof(*argv);
    char **argv_copy = malloc(size);

    memcpy(argv_copy, argv, size);
    printf("\nOPTPARSE\n");
    try_optparse(argv_copy);

    printf("\nOPTPARSE LONG\n");
    try_optparse_long(argv);
    return 0;
}

static int
testsuite(void)
{
    struct config {
        char amend;
        char brief;
        char *color;
        int delay;
        int erase;
    };
    struct {
        char *argv[8];
        struct config conf;
        char *args[8];
        char *err;
    } t[] = {
        {
            {"", "--", "foobar", 0},
            {0, 0, 0, 0, 0},
            {"foobar", 0},
            0
        },
        {
            {"", "-a", "-b", "-c", "-d", "10", "-e", 0},
            {1, 1, "", 10, 1},
            {0},
            0
        },
        {
            {
                "",
                "--amend",
                "--brief",
                "--color",
                "--delay",
                "10",
                "--erase",
                0
            },
            {1, 1, "", 10, 1},
            {0},
            0
        },
        {
            {"", "-a", "-b", "-cred", "-d", "10", "-e", 0},
            {1, 1, "red", 10, 1},
            {0},
            0
        },
        {
            {"", "-abcblue", "-d10", "foobar", 0},
            {1, 1, "blue", 10, 0},
            {"foobar", 0},
            0
        },
        {
            {"", "--color=red", "-d", "10", "--", "foobar", 0},
            {0, 0, "red", 10, 0},
            {"foobar", 0},
            0
        },
        {
            {"", "-eeeeee", 0},
            {0, 0, 0, 0, 6},
            {0},
            0
        },
        {
            {"", "--delay", 0},
            {0, 0, 0, 0, 0},
            {0},
            OPTPARSE_MSG_MISSING
        },
        {
            {"", "--foo", "bar", 0},
            {0, 0, 0, 0, 0},
            {"--foo", "bar", 0},
            OPTPARSE_MSG_INVALID
        },
        {
            {"", "-x", 0},
            {0, 0, 0, 0, 0},
            {"-x", 0},
            OPTPARSE_MSG_INVALID
        },
        {
            {"", "-", 0},
            {0, 0, 0, 0, 0},
            {"-", 0},
            0
        },
        {
            {"", "-e", "foo", "bar", "baz", "-a", "quux", 0},
            {1, 0, 0, 0, 1},
            {"foo", "bar", "baz", "quux", 0},
            0
        },
        {
            {"", "foo", "--delay", "1234", "bar", "-cred", 0},
            {0, 0, "red", 1234, 0},
            {"foo", "bar", 0},
            0
        },
    };
    int ntests = sizeof(t) / sizeof(*t);
    int i, nfails = 0;
    struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_OPTIONAL},
        {"delay", 'd', OPTPARSE_REQUIRED},
        {"erase", 'e', OPTPARSE_NONE},
        {0, 0, 0}
    };

    for (i = 0; i < ntests; i++) {
        int j, opt, longindex;
        char *arg, *err = 0;
        struct optparse options;
        struct config conf = {0, 0, 0, 0, 0};

        optparse_init(&options, t[i].argv);
        while ((opt = optparse_long(&options, longopts, &longindex)) != -1) {
            switch (opt) {
            case 'a': conf.amend = 1; break;
            case 'b': conf.brief = 1; break;
            case 'c': conf.color = options.optarg ? options.optarg : ""; break;
            case 'd': conf.delay = atoi(options.optarg); break;
            case 'e': conf.erase++; break;
            default: err = options.errmsg;
            }
        }

        if (conf.amend != t[i].conf.amend) {
            nfails++;
            printf("FAIL (%2d): expected amend %d, got %d\n",
                   i, t[i].conf.amend, conf.amend);
        }

        if (conf.brief != t[i].conf.brief) {
            nfails++;
            printf("FAIL (%2d): expected brief %d, got %d\n",
                   i, t[i].conf.brief, conf.brief);
        }

        if (t[i].conf.color) {
            if (!conf.color || strcmp(conf.color, t[i].conf.color)) {
                nfails++;
                printf("FAIL (%2d): expected color %s, got %s\n",
                       i, t[i].conf.color, conf.color ? conf.color : "(nil)");
            }
        } else {
            if (conf.color) {
                nfails++;
                printf("FAIL (%2d): expected no color, got %s\n",
                       i, conf.color);
            }
        }

        if (conf.delay != t[i].conf.delay) {
            nfails++;
            printf("FAIL (%2d): expected delay %d, got %d\n",
                   i, t[i].conf.delay, conf.delay);
        }

        if (conf.erase != t[i].conf.erase) {
            nfails++;
            printf("FAIL (%2d): expected erase %d, got %d\n",
                   i, t[i].conf.erase, conf.erase);
        }

        if (t[i].err) {
            if (!err || strncmp(err, t[i].err, strlen(t[i].err))) {
                nfails++;
                printf("FAIL (%2d): expected error '%s', got %s\n",
                       i, t[i].err, err && err[0] ? err : "(nil)");
            }

        } else {
            if (err) {
                nfails++;
                printf("FAIL (%2d): expected no error, got %s\n",
                       i, err);
            }

            for (j = 0; t[i].args[j]; j++) {
                arg = optparse_arg(&options);
                if (!arg || strcmp(arg, t[i].args[j])) {
                    nfails++;
                    printf("FAIL (%2d): expected arg %s, got %s\n",
                           i, t[i].args[j], arg ? arg : "(nil)");
                }
            }
            if ((arg = optparse_arg(&options))) {
                nfails++;
                printf("FAIL (%2d): expected no more args, got %s\n",
                       i, arg);
            }
        }
    }

    if (nfails == 0) {
        puts("All tests pass.");
    }
    return nfails != 0;
}

int
main(int argc, char **argv)
{
    if (argc > 1) {
        return manual_test(argc, argv);
    } else {
        return testsuite();
    }
}
