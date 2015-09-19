CFLAGS = -std=c99 -Wall -Wextra -g3

main : main.o optparse.o

main.o : main.c optparse.h
optparse.o : optparse.c optparse.h

run : main
	./$^ -abdfoo -c bar subcommand example.txt -a

clean :
	$(RM) test *.o
