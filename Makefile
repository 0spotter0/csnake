default:
	gcc -o snake csnake.c -lncurses

clean:
	rm -f *.o snake
