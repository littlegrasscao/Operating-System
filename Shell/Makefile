default: test

test: mysh
	./mysh

valgrind: test_shell
	valgrind --show-reachable=yes --leak-check=full --track-origins=yes -v ./mysh

mysh: shell.o
	gcc -g -std=c11 -Wall -Werror -pedantic shell.o -o mysh

shell.o: shell.c
	gcc -g -std=c11 -Wall -Werror -pedantic -c shell.c

clean:
	rm -f *.o
	rm -f mysh