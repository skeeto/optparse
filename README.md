# Optparse

Optparse is a public domain, portable, reentrant, embeddable, getopt-like
option parser. As a single header file, it's trivially dropped into any
project. It supports POSIX getopt option strings, GNU-style long options,
argument permutation, and subcommand processing.

To get the implementation, define `OPTPARSE_IMPLEMENTATION` before
including `optparse.h`.

~~~c
#define OPTPARSE_IMPLEMENTATION
#include "optparse.h"
~~~

Optionally define `OPTPARSE_API` to control the API's visibility
and/or linkage (`static`, `__attribute__`, `__declspec`).

~~~c
#define OPTPARSE_API static
#include "optparse.h"
~~~

## Why not getopt()?

The POSIX getopt option parser has three fatal flaws. These flaws are
solved by Optparse.

1. The getopt parser state is stored entirely in global variables,
some of which are static and inaccessible. This means only one thread
can use getopt. It also means it's not possible to recursively parse
nested sub-arguments while in the middle of argument parsing. Optparse
fixes this by storing all state on a local struct.

2. The POSIX standard provides no way to properly reset the parser.
For portable code this means getopt is only good for one run, over one
argv with one option string. It also means subcommand options cannot
be reliably processed with getopt. Most implementations provide an
implementation-specific method to reset the parser, but this is not
portable. Optparse provides an `optparse_arg()` function for stepping
through non-option arguments, and parsing of options can continue
again at any time with a different option string. The Optparse struct
itself could be passed around to subcommand handlers for additional
subcommand option parsing. If a full parser reset is needed,
`optparse_init()` can be called again.

3. In getopt, error messages are printed to stderr. This can be
disabled with opterr, but the messages themselves are still
inaccessible. Optparse solves this by writing the error message to its
errmsg field, which can be printed to anywhere. The downside to
Optparse is that this error message will always be in English rather
than the current locale.

## Permutation

By default, argv is permuted as it is parsed, moving non-option
arguments to the end of the array. This can be disabled by setting the
`permute` field to 0 after initialization.

~~~c
struct optparse options;
optparse_init(&options, argv);
options.permute = 0;
~~~

## Drop-in Replacement

Optparse's interface should be familiar with anyone accustomed to
getopt. It's nearly a drop-in replacement. The option string has the
same format and the parser struct fields have the same names as the
getopt global variables (optarg, optind, optopt).

The long option parser `optparse_long()` API is very similar to GNU's
`getopt_long()` and can serve as a portable, embedded replacement.

Optparse does not allocate memory. Furthermore, Optparse has no
dependencies, including libc itself, so it can be used in situations
where the standard C library cannot.

See `optparse.h` for full API documentation.

## Example Usage

Here's a traditional getopt setup.

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>

int main(int argc, char **argv)
{
    bool amend = false;
    bool brief = false;
    const char *color = "white";
    int delay = 0;

    int option;
    while ((option = getopt(argc, argv, "abc:d::")) != -1) {
        switch (option) {
        case 'a':
            amend = true;
            break;
        case 'b':
            brief = true;
            break;
        case 'c':
            color = optarg;
            break;
        case 'd':
            delay = optarg ? atoi(optarg) : 1;
            break;
        case '?':
            exit(EXIT_FAILURE);
        }
    }

    /* Print remaining arguments. */
    for (; optind < argc; optind++)
        printf("%s\n", argv[optind]);
    return 0;
}
~~~

Here's the same thing translated to Optparse.

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

int main(int argc, char **argv)
{
    bool amend = false;
    bool brief = false;
    const char *color = "white";
    int delay = 0;

    char *arg;
    int option;
    struct optparse options;

    optparse_init(&options, argv);
    while ((option = optparse(&options, "abc:d::")) != -1) {
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
~~~

And here's a conversion to long options.

~~~c
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define OPTPARSE_IMPLEMENTATION
#define OPTPARSE_API static
#include "optparse.h"

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
~~~

## Subcommand Parsing

To parse subcommands, first parse options with permutation disabled. These
are the "global" options that come before the subcommand. Then parse the
remainder, optionally permuting, as a new option array.

See `examples/subcommands.c` for a complete, working example.
