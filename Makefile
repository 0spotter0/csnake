default:
	gcc -o snake csnake.c -lncurses

test:
	leaks --atExit -- ./snake $1 $2

clean:
	rm -f *.o snake
