default:
	gcc -o snake csnake.c -lncurses

test:
	leaks --atExit -- ./snake

clean:
	rm -f *.o snake
