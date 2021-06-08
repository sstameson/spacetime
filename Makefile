CC = clang
CFLAGS = -Wall -Werror -fsanitize=address

default: game.c
	$(CC) $(CFLAGS) game.c -o game

clean:
	rm -f game
