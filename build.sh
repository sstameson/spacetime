CC="clang"
CFLAGS="-Wall -Werror -fsanitize=address -lSDL2 -lSDL2_gfx"

$CC $CFLAGS sdl_wrapper.c polygon.c vector.c game.c -o game
