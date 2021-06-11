#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_mixer.h>
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
const f64 MS_PER_SEC = 1000.0;
SDL_Window *window;
SDL_Renderer *renderer;
Mix_Chunk *start;
Mix_Chunk *shoot;
Mix_Chunk *hit;
Mix_Chunk *thrust;
Mix_Chunk *game_over;
static i16 x_points[MAX_POINTS];
static i16 y_points[MAX_POINTS];
static u64 prev_tick = 0;
static KeyHandler key_handler;
static u32 key_start_timestamp;

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
    Mix_Init(MIX_INIT_OGG);
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024);
    Mix_ReserveChannels(1);
    start = Mix_LoadWAV("sounds/start.wav");
    shoot = Mix_LoadWAV("sounds/shoot.wav");
    hit = Mix_LoadWAV("sounds/hit.wav");
    thrust = Mix_LoadWAV("sounds/thrust.wav");
    game_over = Mix_LoadWAV("sounds/game_over.wav");
}

void sdl_play_start(void)
{
    Mix_PlayChannel(-1, start, 0);
}

void sdl_play_shoot(void)
{
    Mix_PlayChannel(-1, shoot, 0);
}

void sdl_play_hit(void)
{
    Mix_PlayChannel(-1, hit, 0);
}

void sdl_play_game_over(void)
{
    Mix_PlayChannel(0, game_over, 0);
}

void sdl_play_thrust(void)
{
    Mix_PlayChannel(0, thrust, -1);
}

void sdl_stop_thrust(void)
{
    Mix_FadeOutChannel(0, 500);
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
            return key == (SDL_Keycode) (u8) key ? key : '\0';
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

                u32 timestamp = event.key.timestamp;
                if (!event.key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event.type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                f64 held_time = (timestamp - key_start_timestamp) / MS_PER_SEC;
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
    for (usize i = 0; i < poly->n; i++) {
        Vector2 v = poly->points[i];
        v = vec_add(v, origin);
        v.y = -v.y + HEIGHT;
        x_points[i] = (i16) v.x;
        y_points[i] = (i16) v.y;
    }

    filledPolygonRGBA(renderer, x_points, y_points, poly->n,
            255 * c.r, 255 * c.g, 255 * c.b, 255 * c.a);
}

void sdl_show(void)
{
    SDL_RenderPresent(renderer);
}

void sdl_quit(void)
{
    Mix_FreeChunk(start);
    Mix_FreeChunk(shoot);
    Mix_FreeChunk(hit);
    Mix_FreeChunk(thrust);
    Mix_FreeChunk(game_over);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

f64 time_since_last_tick(void)
{
    u64 curr_tick = SDL_GetPerformanceCounter();
    f64 diff = prev_tick
        ? (f64) (curr_tick - prev_tick) / (f64) SDL_GetPerformanceFrequency()
        : 0.0;
    prev_tick = curr_tick;
    return diff;
}
