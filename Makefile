default:
	gcc -o bin/snake csnake.c -lncurses

clean:
	rm -f *.o snake
