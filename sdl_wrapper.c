#include <stdbool.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL2_gfxPrimitives.h>

#include "const.h"
#include "vector.h"
#include "polygon.h"
#include "color.h"
#include "sdl_wrapper.h"

const char *WINDOW_TITLE = "Game";
const Vector2 origin = {
    .x = WIDTH / 2.0,
    .y = HEIGHT / 2.0,
};
SDL_Window *window;
SDL_Renderer *renderer;
const double MS_PER_SEC = 1000.0;
static int16_t x_points[MAX_POINTS];
static int16_t y_points[MAX_POINTS];
static uint64_t prev_tick = 0;
static KeyHandler key_handler;
static uint32_t key_start_timestamp;

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

void sdl_on_key(KeyHandler handler)
{
    key_handler = handler;
}

char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return LEFT_ARROW;
        case SDLK_UP: return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN: return DOWN_ARROW;
        case SDLK_RETURN: return ENTER;
        default:
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

bool sdl_running(void *aux)
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
                if (key_handler == NULL) break;

                char key = get_keycode(event.key.keysym.sym);
                if (key == '\0') break;

                uint32_t timestamp = event.key.timestamp;
                if (!event.key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event.type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time = (timestamp - key_start_timestamp) / MS_PER_SEC;
                key_handler(key, type, held_time, aux);
                break;

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

void sdl_draw_polygon(const Polygon *poly, Color c)
{
    for (size_t i = 0; i < poly->n; i++) {
        Vector2 v = poly->points[i];
        v = vec_add(v, origin);
        v.y = -v.y + HEIGHT;
        x_points[i] = (int16_t) v.x;
        y_points[i] = (int16_t) v.y;
    }

    filledPolygonRGBA(renderer, x_points, y_points, poly->n, c.r, c.g, c.b, c.a);
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
    uint64_t curr_tick = SDL_GetPerformanceCounter();
    double diff = prev_tick
        ? (double) (curr_tick - prev_tick) / (double) SDL_GetPerformanceFrequency()
        : 0.0;
    prev_tick = curr_tick;
    return diff;
}
