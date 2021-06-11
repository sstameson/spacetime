CC="clang"
CFLAGS="-Wall -Werror -fsanitize=address -lSDL2 -lSDL2_gfx -lSDL2_mixer"

$CC $CFLAGS sdl_wrapper.c polygon.c vector.c collision.c game.c -o game
