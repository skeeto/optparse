#include <stdio.h>
#include "optparse.h"
#include <getopt.h>

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

void try_optparse(int argc, char **argv)
{
    print_argv(argv);
    struct optparse options;
    optparse_init(&options, argc, argv);
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

void try_optparse_long(int argc, char **argv)
{
    print_argv(argv);
    struct optparse options;
    optparse_init(&options, argc, argv);
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
    printf("GETOPT\n");
    try_getopt(argc, argv);
    printf("\nOPTPARSE\n");
    try_optparse(argc, argv);

    char *long_argv[] = {
        "./main", "--amend", "-b", "--color", "red", "--delay",
        "subcommand", "example.txt", NULL
    };
    printf("\nOPTPARSE LONG\n");
    try_optparse_long(sizeof(long_argv) / sizeof(char *), long_argv);
    return 0;
}
