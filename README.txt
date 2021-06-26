This is an implementation of the game Asteroids with a completely custom
2D physics engine. The game engine draws 2D polygons on the screen using
SDL graphics primitives, draws text with SDL ttf, and  manages sounds with SDL
Mixer, but all physics and collision detection was implemented from scratch.

To build the game, install SDL2, SDL2 ttf, SDL2 mixer, and SDL2 gfx. Then run
build.sh from the root directory of the project.

Notably, the game code uses no dynamic memory allocation. I did this to learn
what it's like to write programs in resource constrained environments where
only static memory allocation is allowed.

The majority of the game code is in src/game.c

Some supporting code for vector/polygon math is in the src/library directory
