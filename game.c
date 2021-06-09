#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "vector.h"
#include "color.h"
#include "const.h"
#include "polygon.h"
#include "sdl_wrapper.h"

const double ASTEROID_VEL = 250.0;
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
    Vector2 points[MAX_POINTS];
    size_t n_points;
    Color color;
    Vector2 v;
    Vector2 a;
    double theta;
    double omega;
} Entity;

typedef struct {
    Entity es[MAX_ENTITIES];
    size_t n;
} GameState;

void spawn_asteroid_with_info(
    GameState *state,
    double r,
    Color color,
    Vector2 cent,
    Vector2 v)
{
    assert(state->n < MAX_ENTITIES);
    if (state->n >= MAX_ENTITIES) {
        return;
    }

    {
        double theta = 0.0;
        double steps[ASTEROID_POINTS];
        double sum = 0.0;
        for (size_t i = 0; i < ASTEROID_POINTS; i++) {
            steps[i] = rand_double(0.0, 1.0);
            sum += steps[i];
        }
        Vector2 v = vec(0.0, r);
        for (size_t i = 0; i < ASTEROID_POINTS; i++) {
            state->es[state->n].points[i] = vec_rotate(theta, v);
            theta += 2.0 * M_PI * (steps[i] / sum);
        }
    }

    state->es[state->n].n_points = ASTEROID_POINTS;
    poly_translate(state->es[state->n].points, state->es[state->n].n_points, cent);
    state->es[state->n].color = color;
    state->es[state->n].v = v;
    state->es[state->n].a = vec(0.0, 0.0);
    state->es[state->n].theta = 0.0;
    state->es[state->n].omega = 0.0;
    state->n += 1;
}

void spawn_asteroid(GameState *state, double r)
{
    const uint8_t i = rand() % (200 - 50) + 50;
    Color c = {.r = i, .g = i, .b = i, .a = 255 };
    double d = rand_double(-1.0, 1.0);
    Vector2 dir = {
        .x = d,
        .y = (rand() % 2 ? -1.0 : 1.0) * sqrt(1.0 - d * d),
    };
    Vector2 cent;
    switch(rand() % 4) {
        case 0:
        {
            cent = vec(MIN.x - r, rand_double(MIN.y, MAX.y));
        } break;
        case 1:
        {
            cent = vec(MAX.x + r, rand_double(MIN.y, MAX.y));
        } break;
        case 2:
        {
            cent = vec(rand_double(MIN.x, MAX.x), MIN.y - r);
        } break;
        case 3:
        {
            cent = vec(rand_double(MIN.x, MAX.x), MAX.y + r);
        } break;
    }
    spawn_asteroid_with_info(state, r, c, cent, vec_mul(ASTEROID_VEL, dir));
}

void init(GameState *state)
{
    state->n = 0;
    for (size_t i = 0; i < 5; i++) {
        spawn_asteroid(state, 100.0);
    }
}

void update(GameState *state, double dt)
{
    for (size_t i = 0; i < state->n; i++) {
        {
            Vector2 min = poly_min(state->es[i].points, state->es[i].n_points);
            Vector2 max = poly_max(state->es[i].points, state->es[i].n_points);
            if (max.x < MIN.x && state->es[i].v.x < 0.0) {
                poly_translate(state->es[i].points, state->es[i].n_points,
                               vec((MAX.x - MIN.x) + (max.x - min.x), 0.0));
            } else if (max.y < MIN.y && state->es[i].v.y < 0.0) {
                poly_translate(state->es[i].points, state->es[i].n_points,
                               vec(0.0, (MAX.y - MIN.y) + (max.y - min.y)));
            } else if (min.x > MAX.x && state->es[i].v.x > 0.0) {
                poly_translate(state->es[i].points, state->es[i].n_points,
                               vec(-(MAX.x - MIN.x) - (max.x - min.x), 0.0));
            } else if (min.y > MAX.y && state->es[i].v.y > 0.0) {
                poly_translate(state->es[i].points, state->es[i].n_points,
                               vec(0.0, -(MAX.y - MIN.y) - (max.y - min.y)));
            }
        }

        poly_translate(state->es[i].points, state->es[i].n_points, vec_mul(dt, state->es[i].v));
    }
}

void render(GameState *state)
{
    sdl_clear();
    for (size_t i = 0; i < state->n; i++) {
        sdl_draw_polygon(
            state->es[i].points, state->es[i].n_points, state->es[i].color);
    }
    sdl_show();
}

int main(void)
{
    sdl_init();
    static GameState state;
    init(&state);
    double t = 0.0;
    size_t frames = 0;

    while (sdl_running()) {
        double dt = time_since_last_tick();
        t += dt;
        frames++;
        update(&state, dt);
        render(&state);
    }

    printf("%f fps\n", (double) frames / t);
    sdl_quit();
}
