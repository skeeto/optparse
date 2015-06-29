#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include "optparse.h"

void print_argv(char **argv)
{
    while (*argv)
        printf("%s ", *argv++);
    printf("\n");
}

void try_getopt(int argc, char **argv)
{
    print_argv(argv);
    int opt;
    while ((opt = getopt(argc, argv, "abc:d::")) != -1) {
        printf("%c (%d) = '%s'\n", opt, optind, optarg);
    }
    printf("optind = %d\n", optind);
    for (; optind < argc; optind++)
        printf("argument: %s\n", argv[optind]);
}

void try_optparse(char **argv)
{
    print_argv(argv);
    struct optparse options;
    optparse_init(&options, argv);
    int opt;
    while ((opt = optparse(&options, "abc:d::")) != -1) {
        if (opt == '?')
            printf("%s: %s\n", argv[0], options.errmsg);
        printf("%c (%d) = '%s'\n", opt, options.optind, options.optarg);
    }
    printf("optind = %d\n", options.optind);
    char *arg;
    while ((arg = optparse_arg(&options)))
        printf("argument: %s\n", arg);
}

void try_optparse_long(char **argv)
{
    print_argv(argv);
    struct optparse options;
    optparse_init(&options, argv);
    struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_REQUIRED},
        {"delay", 'd', OPTPARSE_OPTIONAL},
        {0}
    };
    int opt, longindex;
    while ((opt = optparse_long(&options, longopts, &longindex)) != -1) {
        if (opt == '?')
            printf("%s: %s\n", argv[0], options.errmsg);
        printf("%c (%d, %d) = '%s'\n",
               opt, options.optind, longindex, options.optarg);
    }
    printf("optind = %d\n", options.optind);
    char *arg;
    while ((arg = optparse_arg(&options)))
        printf("argument: %s\n", arg);
}

int main(int argc, char **argv)
{
    char *argv_copy[argc + 1];
    memcpy(argv_copy, argv, sizeof(argv_copy));
    printf("GETOPT\n");
    try_getopt(argc, argv_copy);

    memcpy(argv_copy, argv, sizeof(argv_copy));
    printf("\nOPTPARSE\n");
    try_optparse(argv_copy);

    char *long_argv[] = {
        "./main", "--amend", "-b", "--color", "red", "--delay=22",
        "subcommand", "example.txt", "--amend", NULL
    };
    printf("\nOPTPARSE LONG\n");
    try_optparse_long(long_argv);
    return 0;
}
