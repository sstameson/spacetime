#ifndef _SDL_WRAPPER_H_
#define _SDL_WRAPPER_H_

typedef enum {
    LEFT_ARROW = 1,
    UP_ARROW = 2,
    RIGHT_ARROW = 3,
    DOWN_ARROW = 4,
    ENTER = 5
} ArrowKey;

typedef enum {
    KEY_PRESSED,
    KEY_RELEASED
} KeyEventType;

typedef void (*KeyHandler)(char key, KeyEventType type, double held_time, void *aux);

void sdl_play_shoot(void);

void sdl_play_hit(void);

void sdl_play_thrust(void);

void sdl_stop_thrust(void);

void sdl_init(void);

void sdl_on_key(KeyHandler handler);

bool sdl_running(void *aux);

void sdl_clear(void);

void sdl_draw_polygon(const Polygon *poly, Color c);

void sdl_show(void);

void sdl_quit(void);

double time_since_last_tick(void);

#endif
