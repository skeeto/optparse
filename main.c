#include <stdio.h>
#include "optparse.h"
#include <getopt.h>

void try_getopt(int argc, char **argv)
{
    int opt;
    while ((opt = getopt(argc, argv, "abc:d::")) != -1) {
        printf("%c (%d) = '%s'\n", opt, optind, optarg);
    }
    printf("optind = %d\n", optind);
}

void try_optparse(int argc, char **argv)
{
    struct optparse options;
    optparse_init(&options, argc, argv);
    int opt;
    while ((opt = optparse(&options, "abc:d::")) != -1) {
        if (opt == '?')
            printf("%s: %s\n", argv[0], options.errmsg);
        printf("%c (%d) = '%s'\n", opt, options.optind, options.optarg);
    }
    printf("optind = %d\n", options.optind);
}

int main(int argc, char **argv)
{
    printf("GETOPT\n");
    try_getopt(argc, argv);
    printf("\nOPTPARSE\n");
    try_optparse(argc, argv);
    return 0;
}
