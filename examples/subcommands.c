/* This is free and unencumbered software released into the public domain. */
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OPTPARSE_IMPLEMENTATION
#include "../optparse.h"

static int cmd_echo(char **argv)
{
    int i, option;
    bool newline = true;
    struct optparse options;

    optparse_init(&options, argv);
    options.permute = 0;
    while ((option = optparse(&options, "?hn")) != -1) {
        switch (option) {
        case '?':
        case 'h':
            puts("usage: echo [-?hn] [ARG]...");
            return EXIT_SUCCESS;
        case 'n':
            newline = false;
            break;
        default:
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            return EXIT_FAILURE;
        }
    }
    argv += options.optind;

    for (i = 0; argv[i]; i++) {
        printf("%s%s", i  ? " " : "", argv[i]);
    }
    if (newline) {
        putchar('\n');
    }

    fflush(stdout);
    return !!ferror(stdout);
}

static int cmd_sleep(char **argv)
{
    int i, option;
    struct optparse options;

    optparse_init(&options, argv);
    while ((option = optparse(&options, "?h")) != -1) {
        switch (option) {
        case '?':
        case 'h':
            puts("usage: sleep [-?h] [NUMBER]...");
            return EXIT_SUCCESS;
        default:
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            return EXIT_FAILURE;
        }
    }

    for (i = 0; argv[i]; i++) {
        if (sleep(atoi(argv[i]))) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}

static void
usage(FILE *f)
{
    fprintf(f, "usage: example [-?h] <echo|sleep> [OPTION]...\n");
}

int main(int argc, char **argv)
{
    int i, option;
    char **subargv;
    struct optparse options;

    static const struct {
        char name[8];
        int (*cmd)(char **);
    } cmds[] = {
        {"echo",  cmd_echo },
        {"sleep", cmd_sleep},
    };
    int ncmds = sizeof(cmds) / sizeof(*cmds);

    (void)argc;
    optparse_init(&options, argv);
    options.permute = 0;
    while ((option = optparse(&options, "?h")) != -1) {
        switch (option) {
        case '?':
        case 'h':
            usage(stdout);
            return EXIT_SUCCESS;
        default:
            usage(stderr);
            fprintf(stderr, "%s: %s\n", argv[0], options.errmsg);
            return EXIT_FAILURE;
        }
    }

    subargv = argv + options.optind;
    if (!subargv[0]) {
        fprintf(stderr, "%s: missing subcommand\n", argv[0]);
        usage(stderr);
        return EXIT_FAILURE;
    }

    for (i = 0; i < ncmds; i++) {
        if (!strcmp(cmds[i].name, subargv[0])) {
            return cmds[i].cmd(subargv);
        }
    }
    fprintf(stderr, "%s: invalid subcommand: %s\n", argv[0], subargv[0]);
    return EXIT_FAILURE;
}
