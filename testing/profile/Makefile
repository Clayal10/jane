all:
	gcc main.c ../../src/*.c -g

profile:
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes -s ./a.out 5000

test:
	./a.out 5000

clean:
	rm a.out
