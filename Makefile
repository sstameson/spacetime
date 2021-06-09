CC = clang
CFLAGS = -Wall -Werror -fsanitize=address -lSDL2 -lSDL2_gfx

default:
	$(CC) $(CFLAGS) sdl_wrapper.c vector.c game.c -o game

clean:
	rm -f game
