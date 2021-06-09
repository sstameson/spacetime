#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "vector.h"
#include "color.h"
#include "const.h"
#include "polygon.h"
#include "sdl_wrapper.h"

const Vector2 MAX = {
    .x = WIDTH / 2.0,
    .y = HEIGHT / 2.0,
};
const Vector2 MIN = {
    .x = -WIDTH / 2.0,
    .y = -HEIGHT / 2.0,
};

double rand_double(double min, double max)
{
    return (max - min) * (double) rand() / (double) RAND_MAX + min;
}

typedef struct {
    Vector2 poly[MAX_POLYS][MAX_POLY_POINTS];
    size_t n_verts[MAX_POLYS];
    Color color[MAX_POLYS];
    Vector2 v[MAX_POLYS];
    Vector2 a[MAX_POLYS];
    double theta[MAX_POLYS];
    double omega[MAX_POLYS];
    size_t n;
} GameState;

void spawn_asteroid(GameState *state, double r) {
    {
        double theta = 0.0;
        double steps[ASTEROID_N_SIDES];
        double sum = 0.0;
        for (size_t i = 0; i < ASTEROID_N_SIDES; i++) {
            steps[i] = rand_double(0.0, 1.0);
            sum += steps[i];
        }
        Vector2 v = vec(0.0, r);
        for (size_t i = 0; i < ASTEROID_N_SIDES; i++) {
            state->poly[state->n][i] = vec_rotate(theta, v);
            theta += 2.0 * M_PI * (steps[i] / sum);
        }
    }

    state->n_verts[state->n] = ASTEROID_N_SIDES;

    {
        Vector2 cent = {
            .x = rand_double(MIN.x, MAX.x),
            .y = rand_double(MIN.x, MAX.x),
        };
        translate_poly(state->poly[state->n], state->n_verts[state->n], cent);
    }

    {
        const uint8_t x = rand() % (200 - 50) + 50;
        Color c = {
            .r = x,
            .g = x,
            .b = x,
            .a = 255,
        };
        state->color[state->n] = c;
    }

    {
        double x = rand_double(-1.0, 1.0);
        Vector2 dir = {
            .x = x,
            .y = (rand() % 2 ? -1.0 : 1.0) * sqrt(1.0 - x * x),
        };
        state->v[state->n] = vec_mul(30.0, dir);
    }

    state->a[state->n] = vec(0.0, 0.0);
    state->theta[state->n] = 0.0;
    state->omega[state->n] = 0.0;
    state->n += 1;
}

void update(GameState *state, double dt) {
    for (size_t i = 0; i < state->n; i++) {
        Vector2 prev_v = state->v[i];
        state->v[i] = vec_add(state->v[i], vec_mul(dt, state->a[i]));
        Vector2 v = vec_mul(0.5, vec_add(prev_v, state->v[i]));

        translate_poly(state->poly[i], state->n_verts[i], vec_mul(dt, v));
    }
}

void render(GameState *state) {
    sdl_clear();
    for (size_t i = 0; i < state->n; i++) {
        sdl_draw_polygon(state->poly[i], state->n_verts[i], state->color[i]);
    }
    sdl_show();
}

int main(void)
{
    static GameState state;

    sdl_init();

    for (size_t i = 0; i < 10; ++i) {
        spawn_asteroid(&state, 100.0);
    }

    while (sdl_running()) {
        update(&state, time_since_last_tick());
        render(&state);
    }

    sdl_quit();
}
