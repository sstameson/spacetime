#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "const.h"
#include "vector.h"
#include "color.h"
#include "sdl_wrapper.h"

const char *WINDOW_TITLE = "Game";

const Vector2 origin = {
    .x = WIDTH / 2.0,
    .y = HEIGHT / 2.0,
};
SDL_Window *window;
SDL_Renderer *renderer;
clock_t last_clock;
static int16_t x_points[MAX_POLY_POINTS];
static int16_t y_points[MAX_POLY_POINTS];

void sdl_init(void)
{
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr, "Unable to initialize SDL! Exiting...\n");
        exit(1);
    }
    SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengl");
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        WIDTH,
        HEIGHT,
        0);
    renderer = SDL_CreateRenderer(window, -1, 0);
}

bool sdl_running(void)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch(event.type) {
            case SDL_QUIT:
            {
                return false;
            } break;

            case SDL_KEYUP:
            case SDL_KEYDOWN:
            {
                // TODO
            } break;
        }
    }
    return true;
}

void sdl_clear(void)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

void sdl_draw_polygon(Vector2 *poly, size_t n, Color c)
{
    for (size_t i = 0; i < n; i++) {
        Vector2 v = poly[i];
        v = vec_add(v, origin);
        v.y = -v.y + HEIGHT;
        x_points[i] = (int16_t) v.x;
        y_points[i] = (int16_t) v.y;
    }

    filledPolygonRGBA(renderer, x_points, y_points, n, c.r, c.g, c.b, c.a);
}

void sdl_show(void)
{
    SDL_RenderPresent(renderer);
}

void sdl_quit(void)
{
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

double time_since_last_tick(void)
{
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}
