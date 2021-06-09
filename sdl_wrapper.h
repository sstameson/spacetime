#ifndef _SDL_WRAPPER_H_
#define _SDL_WRAPPER_H_

void sdl_init(void);

bool sdl_running(void);

void sdl_clear(void);

void sdl_draw_polygon(Vector2 *poly, size_t n, Color c);

void sdl_show(void);

void sdl_quit(void);

double time_since_last_tick(void);

#endif
