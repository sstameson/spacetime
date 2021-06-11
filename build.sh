CC="clang"
CFLAGS="-Wall -Werror -fsanitize=address -lSDL2 -lSDL2_gfx -lSDL2_mixer -Isrc/include"
LIB="src/library/"
CFILES="${LIB}sdl_wrapper.c ${LIB}polygon.c ${LIB}vector.c ${LIB}collision.c src/game.c"

$CC $CFLAGS $CFILES -o game
