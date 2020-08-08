CFLAGS = -ansi -pedantic -Wall -Wextra -g3

test : test.c optparse.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ test.c $(LDLIBS)

run : test
	./test -abdfoo -c bar subcommand example.txt -a

clean :
	rm -f test
