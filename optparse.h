#ifndef OPTPARSE_H
#define OPTPARSE_H

/**
 * Portable, re-entrant getopt() alternative.
 */

struct optparse {
    int argc;
    char **argv;
    int optind;
    int subopt;
    const char *optarg;
    char errmsg[48];
};

void  optparse_init(struct optparse *opts, int argc, char **argv);
int   optparse(struct optparse *opts, const char *spec);
char *optparse_arg(struct optparse *opts);

#endif
