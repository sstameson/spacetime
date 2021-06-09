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
    Vector2 cent;
    Vector2 v;
    Vector2 a;
    double theta;
    double omega;
    bool removed;
} Entity;

typedef size_t EntityIndex;

typedef struct {
    EntityIndex idxs[MAX_ENTITIES];
    size_t length;
} EntityIndexArray;

typedef struct {
    Entity entities[MAX_ENTITIES];
    EntityIndex player;
    EntityIndexArray asteroids;
    EntityIndexArray bullets;
    EntityIndexArray particles;
} GameState;

void push(EntityIndexArray *arr, EntityIndex idx)
{
    assert(arr->length < MAX_ENTITIES);
    arr->idxs[arr->length] = idx;
    arr->length += 1;
}

EntityIndex alloc_entity(Entity entities[MAX_ENTITIES])
{
    for (size_t i = 0; i < MAX_ENTITIES; i++) {
        if (entities[i].removed) {
            entities[i].removed = false;
            return i;
        }
    }
    fprintf(stderr, "Failed to allocate entity! Exiting...\n");
    exit(1);
}

void free_entity(Entity entities[MAX_ENTITIES], EntityIndex idx)
{
    // TODO: What to do about invalid IDs in Index arrays?
    entities[idx].removed = true;
}

void entity_translate(GameState *state, EntityIndex idx, Vector2 t)
{
    poly_translate(state->entities[idx].points, state->entities[idx].n_points, t);
    state->entities[idx].cent = vec_add(state->entities[idx].cent, t);
}

void entity_rotate(GameState *state, EntityIndex idx, double theta)
{
    poly_rotate(
        state->entities[idx].points,
        state->entities[idx].n_points,
        theta,
        state->entities[idx].cent);
    state->entities[idx].theta += theta;
}

void spawn_asteroid_with_info(
    GameState *state,
    double r,
    Color color,
    Vector2 cent,
    Vector2 v)
{
    EntityIndex idx = alloc_entity(state->entities);

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
            state->entities[idx].points[i] = vec_rotate(theta, v);
            theta += 2.0 * M_PI * (steps[i] / sum);
        }
    }

    state->entities[idx].n_points = ASTEROID_POINTS;
    state->entities[idx].color = color;
    state->entities[idx].cent = poly_centroid(
        state->entities[idx].points, state->entities[idx].n_points);
    {
        Vector2 t = vec_sub(cent, state->entities[idx].cent);
        entity_translate(state, idx, t);
    }
    state->entities[idx].v = v;
    state->entities[idx].a = vec(0.0, 0.0);
    state->entities[idx].theta = 0.0;
    state->entities[idx].omega = 1.0;
    push(&state->asteroids, idx);
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
    for (size_t i = 0; i < MAX_ENTITIES; i++) {
        free_entity(state->entities, i);
    }

    for (size_t i = 0; i < 5; i++) {
        spawn_asteroid(state, 100.0);
    }
}

void teleport(GameState *state, EntityIndex idx)
{
    Vector2 min = poly_min(
        state->entities[idx].points, state->entities[idx].n_points);
    Vector2 max = poly_max(
        state->entities[idx].points, state->entities[idx].n_points);

    if (max.x < MIN.x && state->entities[idx].v.x < 0.0) {

        Vector2 t = vec((MAX.x - MIN.x) + (max.x - min.x), 0.0);
        entity_translate(state, idx, t);

    } else if (max.y < MIN.y && state->entities[idx].v.y < 0.0) {

        Vector2 t = vec(0.0, (MAX.y - MIN.y) + (max.y - min.y));
        entity_translate(state, idx, t);

    } else if (min.x > MAX.x && state->entities[idx].v.x > 0.0) {

        Vector2 t = vec(-(MAX.x - MIN.x) - (max.x - min.x), 0.0);
        entity_translate(state, idx, t);

    } else if (min.y > MAX.y && state->entities[idx].v.y > 0.0) {

        Vector2 t = vec(0.0, -(MAX.y - MIN.y) - (max.y - min.y));
        entity_translate(state, idx, t);
    }
}

void tick(GameState *state, EntityIndex idx, double dt)
{
    state->entities[idx].v = vec_add(
            state->entities[idx].v, vec_mul(dt, state->entities[idx].a));
    entity_translate(state, idx, vec_mul(dt, state->entities[idx].v));
    entity_rotate(state, idx, dt * state->entities[idx].omega);
}

void update(GameState *state, double dt)
{
    for (size_t i = 0; i < state->asteroids.length; i++) {
        EntityIndex idx = state->asteroids.idxs[i];
        teleport(state, idx);
        tick(state, idx, dt);
    }
}

void render(GameState *state)
{
    sdl_clear();
    for (size_t i = 0; i < state->asteroids.length; i++) {
        EntityIndex idx = state->asteroids.idxs[i];
        sdl_draw_polygon(
            state->entities[idx].points,
            state->entities[idx].n_points,
            state->entities[idx].color);
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
