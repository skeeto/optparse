#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

void print_argv(const char **argv)
{
    while (*argv)
        printf("%s ", *argv++);
    printf("\n");
}

void try_getopt(int argc, char **argv)
{
    int opt;

    print_argv((const char **)argv);
    while ((opt = getopt(argc, argv, "abc:d::")) != -1) {
        printf("%c (%d) = '%s'\n", opt, optind, optarg);
    }
    printf("optind = %d\n", optind);
    for (; optind < argc; optind++)
        printf("argument: %s\n", argv[optind]);
}

void try_optparse( const char **argv)
{
    int opt;
    const char *arg;
    struct optparse options;

    print_argv(argv);
    optparse_init(&options, argv);
    while ((opt = optparse(&options, "abc:d::")) != -1) {
        if (opt == '?')
            printf("%s: %s\n", argv[0], options.errmsg);
        printf("%c (%d) = '%s'\n", opt, options.optind, options.optarg);
    }
    printf("optind = %d\n", options.optind);
    while ((arg = optparse_arg(&options)))
        printf("argument: %s\n", arg);
}

void try_optparse_long( const char **argv)
{
    const char *arg;
    int opt, longindex;
    struct optparse options;
    const struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_REQUIRED},
        {"delay", 'd', OPTPARSE_OPTIONAL},
        {0, 0, 0}
    };

    print_argv(argv);
    optparse_init(&options, argv);
    while ((opt = optparse_long(&options, longopts, &longindex)) != -1) {
        if (opt == '?')
            printf("%s: %s\n", argv[0], options.errmsg);
        printf("%c (%d, %d) = '%s'\n",
               opt, options.optind, longindex, options.optarg);
    }
    printf("optind = %d\n", options.optind);
    while ((arg = optparse_arg(&options)))
        printf("argument: %s\n", arg);
}

int main(int argc, char **argv)
{
#ifdef EXAMPLE
    const char *long_argv[] = {
        "./main", "--amend", "-b", "--color", "red", "--delay=22",
        "subcommand", "example.txt", "--amend", NULL
    };
#endif
    size_t size = (argc + 1) * sizeof(*argv);
    char **argv_copy = malloc(size);

    memcpy(argv_copy, argv, size);
    printf("GETOPT\n");
    try_getopt(argc, argv_copy);

    memcpy(argv_copy, argv, size);
    printf("\nOPTPARSE\n");
    try_optparse( (const char **)argv_copy);

    memcpy(argv_copy, argv, size);
    printf("\nOPTPARSE LONG\n");
    try_optparse_long( (const char **)argv_copy);
    return 0;
}
