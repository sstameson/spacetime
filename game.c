#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "vector.h"
#include "polygon.h"
#include "color.h"

#define ASTEROID_N_SIDES 10

const size_t MAX_POLYS = 1000;
const Color BLACK = {
    .r = 0,
    .g = 0,
    .b = 0,
};

typedef struct {
    Vector2 **poly;
    size_t *n_verts;
    Vector2 *v;
    Vector2 *a;
    double *theta;
    double *omega;
    Color *color;
    size_t n;
} GameState;

GameState *init_state(void) {
    GameState *state = malloc(sizeof(GameState));
    state->poly = malloc(MAX_POLYS * sizeof(Vector2 *));
    state->n_verts = malloc(MAX_POLYS * sizeof(size_t));
    state->v = malloc(MAX_POLYS * sizeof(Vector2));
    state->a = malloc(MAX_POLYS * sizeof(Vector2));
    state->theta = malloc(MAX_POLYS * sizeof(double));
    state->omega = malloc(MAX_POLYS * sizeof(double));
    state->color = malloc(MAX_POLYS * sizeof(Color));
    state->n = 0;
    return state;
}

void free_state(GameState *state) {
    for (size_t i = 0; i < state->n; i++) {
        free(state->poly[i]);
    }
    free(state->poly);
    free(state->n_verts);
    free(state->v);
    free(state->a);
    free(state->theta);
    free(state->omega);
    free(state->color);
    free(state);
}

Vector2 *asteroid_poly(double r) {
    double theta = 0.0;
    double steps[ASTEROID_N_SIDES];
    double sum = 0.0;

    for (size_t i = 0; i < ASTEROID_N_SIDES; i++) {
        steps[i] = (double) rand() / (double) RAND_MAX;
        sum += steps[i];
    }

    Vector2 v = vec(0.0, r);
    Vector2 *asteroid = malloc(ASTEROID_N_SIDES * sizeof(Vector2));

    for (size_t i = 0; i < ASTEROID_N_SIDES; i++) {
        asteroid[i] = vec_rotate(theta, v);
        theta += 2.0 * M_PI * (steps[i] / sum);
    }

    return asteroid;
}

void spawn_poly(
    GameState *state,
    Vector2 *poly,
    size_t n_verts,
    Vector2 v,
    Vector2 a,
    double theta,
    double omega,
    Color color)
{
    if (state->n >= MAX_POLYS) {
        fprintf(stderr, "Out of memory! Exiting...\n");
        free_state(state);
        exit(1);
    }
    state->poly[state->n] = poly;
    state->n_verts[state->n] = n_verts;
    state->v[state->n] = v;
    state->a[state->n] = a;
    state->theta[state->n] = theta;
    state->omega[state->n] = omega;
    state->color[state->n] = color;
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
    /* TODO */
}

int main(void)
{
    GameState *state = init_state();
    spawn_poly(
        state, asteroid_poly(25.0), ASTEROID_N_SIDES,
        vec(0.0, 0.0), vec(0.0, 0.0), 0.0, 0.0, BLACK);

    for (size_t i = 0; i < 100000; i++) {
        update(state, 1e-3);
    }

    free_state(state);
}
