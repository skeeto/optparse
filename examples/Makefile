CC      = cc
CFLAGS  = -ansi -pedantic -Wall -Wextra -Wno-unused-function -Wno-unused-but-set-variable -g3
LDFLAGS =
LDLIBS  =

all: short$(EXE) long$(EXE) subcommands$(EXE)

short$(EXE): short.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ short.c ../optparse.h $(LDLIBS)

long$(EXE): long.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ long.c ../optparse.h $(LDLIBS)

subcommands$(EXE): subcommands.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ subcommands.c ../optparse.h $(LDLIBS)

clean:
	rm -f short$(EXE) long$(EXE) subcommands$(EXE)
