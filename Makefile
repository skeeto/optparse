CFLAGS = -std=c99 -Wall -g3

main : main.o optparse.o

main.o : main.c optparse.h
optparse.o : optparse.c optparse.h

run : main
	./$^ -abdfoo -c bar -- subcommand example.txt

clean :
	$(RM) test *.o
