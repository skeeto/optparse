CFLAGS = -ansi -pedantic -Wall -Wextra -g3

main : main.c optparse.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ main.c $(LDLIBS)

run : main
	./$^ -abdfoo -c bar subcommand example.txt -a

clean :
	rm -f main
