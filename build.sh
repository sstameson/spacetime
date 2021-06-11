CC="clang"

CFLAGS="-Wall -Werror -fsanitize=address "
CFLAGS+="-lSDL2 -lSDL2_gfx -lSDL2_mixer -Isrc/include"

BASE="src/library/"
CFILES="${BASE}sdl_wrapper.c "
CFILES+="${BASE}polygon.c "
CFILES+="${BASE}vector.c "
CFILES+="${BASE}collision.c "
CFILES+="src/game.c"

$CC $CFLAGS $CFILES -o game
