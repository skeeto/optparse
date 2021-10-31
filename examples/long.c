/* This is free and unencumbered software released into the public domain. */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "../optparse.h"

int main(int argc, char **argv)
{
    struct optparse_long longopts[] = {
        {"amend", 'a', OPTPARSE_NONE},
        {"brief", 'b', OPTPARSE_NONE},
        {"color", 'c', OPTPARSE_REQUIRED},
        {"delay", 'd', OPTPARSE_OPTIONAL},
        {0}
    };

    bool amend = false;
    bool brief = false;
    const char *color = "white";
    int delay = 0;

    char *arg;
    int option;
    struct optparse options;

    (void)argc;
    optparse_init(&options, argv);
    while ((option = optparse_long(&options, longopts, NULL)) != -1) {
        switch (option) {
        case 'a':
            amend = true;
            break;
        case 'b':
            brief = true;
            break;
        case 'c':
            color = options.optarg;
            break;
        case 'd':
            delay = options.optarg ? atoi(options.optarg) : 1;
            break;
        case '?':
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            exit(EXIT_FAILURE);
        }
    }

    /* Print remaining arguments. */
    while ((arg = optparse_arg(&options)))
        printf("%s\n", arg);

    return 0;
}
