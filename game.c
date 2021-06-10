#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "vector.h"
#include "color.h"
#include "const.h"
#include "collision.h"
#include "polygon.h"
#include "sdl_wrapper.h"

const double PLAYER_LENGTH = 80.0;
const double PLAYER_WIDTH = 40.0;
const double PLAYER_PROP = 0.75;
const size_t BULLET_POINTS = 10;
const double BULLET_RAD = 5.0;
const double BULLET_VEL = 600.0;
const double BIG_ASTEROID_RAD = 60.0;
const double ASTEROID_RAD = 30.0;
const double ASTEROID_VEL = 250.0;
const uint8_t MIN_GREY = 50;
const uint8_t MAX_GREY = 200;
const double THRUST = 750.0;
const double DRAG = 1.0;
const double PLAYER_OMEGA = 1.25 * M_PI;
const Vector2 MAX = {
    .x = WIDTH / 2.0,
    .y = HEIGHT / 2.0,
};
const Vector2 MIN = {
    .x = -WIDTH / 2.0,
    .y = -HEIGHT / 2.0,
};
const Color BLACK = { .r = 0, .g = 0, .b = 0, .a = 255 };
const Color RED = { .r = 255, .g = 0, .b = 0, .a = 255 };

double rand_double(double min, double max)
{
    return (max - min) * (double) rand() / (double) RAND_MAX + min;
}

Vector2 rand_dir(void)
{
    double d = rand_double(-1.0, 1.0);
    Vector2 dir = {
        .x = d,
        .y = (rand() % 2 ? -1.0 : 1.0) * sqrt(1.0 - d * d),
    };
    return dir;
}

typedef struct {
    Polygon poly;
    Color color;
    Vector2 cent;
    Vector2 v;
    Vector2 a;
    double theta;
    double omega;
    uint8_t health;
} Entity;

typedef size_t EntityIndex;

typedef struct {
    EntityIndex idxs[MAX_ENTITIES];
    size_t length;
} EntityIndexArray;

typedef enum {
    START,
    PLAYING,
    OVER,
} GameStatus;

typedef struct {
    GameStatus status;
    bool quiting;
    bool restarting;
    bool thrusting;
    bool turning_clockwise;
    bool turning_counterclockwise;
    bool shooting;
} InputState;

typedef struct {
    Entity entities[MAX_ENTITIES];
    bool free[MAX_ENTITIES];
    EntityIndex player;
    EntityIndexArray asteroids;
    EntityIndexArray bullets;
    EntityIndexArray particles;
    InputState input;
} GameState;

void push(EntityIndexArray *arr, EntityIndex idx)
{
    assert(arr->length < MAX_ENTITIES);
    arr->idxs[arr->length] = idx;
    arr->length += 1;
}

void remove_index(EntityIndexArray *arr, size_t idx)
{
    assert(idx < arr->length);
    assert(arr->length > 0);
    for (size_t i = idx; i < arr->length-1; i++) {
        arr->idxs[i] = arr->idxs[i+1];
    }
    arr->length -= 1;
}

void clear(EntityIndexArray *arr)
{
    arr->length = 0;
}

EntityIndex alloc_entity(bool free[MAX_ENTITIES])
{
    for (size_t i = 0; i < MAX_ENTITIES; i++) {
        if (free[i]) {
            free[i] = false;
            return i;
        }
    }
    // TODO: Make the entities array dynamic
    fprintf(stderr, "Failed to allocate entity! Exiting...\n");
    exit(1);
}

void free_entity(bool free[MAX_ENTITIES], EntityIndex idx)
{
    free[idx] = true;
}

void entity_translate(Entity *entity, Vector2 t)
{
    poly_translate(&entity->poly, t);
    entity->cent = vec_add(entity->cent, t);
}

void entity_rotate(Entity *entity, double theta)
{
    poly_rotate(&entity->poly, theta, entity->cent);
    entity->theta += theta;
}

void entity_tick(Entity *entity, double dt)
{
    entity->v = vec_add(entity->v, vec_mul(dt, entity->a));
    entity_translate(entity, vec_mul(dt, entity->v));
    entity_rotate(entity, dt * entity->omega);
}

void spawn_asteroid_with_info(
    GameState *state,
    double r,
    Color color,
    Vector2 cent,
    Vector2 v,
    uint8_t health)
{
    EntityIndex idx = alloc_entity(state->free);
    Entity *entity = &state->entities[idx];
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
            entity->poly.points[i] = vec_rotate(theta, v);
            theta += 2.0 * M_PI * (steps[i] / sum);
        }
        entity->poly.n = ASTEROID_POINTS;
    }
    entity->color = color;
    entity->cent = poly_centroid(&entity->poly);
    {
        Vector2 t = vec_sub(cent, entity->cent);
        entity_translate(entity, t);
    }
    entity->v = v;
    entity->a = vec(0.0, 0.0);
    entity->theta = 0.0;
    entity->omega = 0.0;
    entity->health = health;
    push(&state->asteroids, idx);
}

void spawn_asteroid(GameState *state)
{
    const uint8_t i = rand() % (MAX_GREY - MIN_GREY) + MIN_GREY;
    Color c = {.r = i, .g = i, .b = i, .a = 255 };
    double r;
    uint8_t health;
    if (rand() % 2) {
        r = BIG_ASTEROID_RAD;
        health = 2;
    } else {
        r = ASTEROID_RAD;
        health = 1;
    }
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
    spawn_asteroid_with_info(
            state, r, c, cent, vec_mul(ASTEROID_VEL, rand_dir()), health);
}

void teleport(Entity *entity)
{
    Vector2 min = poly_min(&entity->poly);
    Vector2 max = poly_max(&entity->poly);

    if (max.x < MIN.x && entity->v.x < 0.0) {

        Vector2 t = vec((MAX.x - MIN.x) + (max.x - min.x), 0.0);
        entity_translate(entity, t);

    } else if (max.y < MIN.y && entity->v.y < 0.0) {

        Vector2 t = vec(0.0, (MAX.y - MIN.y) + (max.y - min.y));
        entity_translate(entity, t);

    } else if (min.x > MAX.x && entity->v.x > 0.0) {

        Vector2 t = vec(-(MAX.x - MIN.x) - (max.x - min.x), 0.0);
        entity_translate(entity, t);

    } else if (min.y > MAX.y && entity->v.y > 0.0) {

        Vector2 t = vec(0.0, -(MAX.y - MIN.y) - (max.y - min.y));
        entity_translate(entity, t);
    }
}

void init_game(GameState *state)
{
    // Free all existing entities
    for (size_t i = 0; i < MAX_ENTITIES; i++) {
        free_entity(state->free, i);
    }
    clear(&state->asteroids);
    clear(&state->bullets);
    clear(&state->particles);

    // Spawn player
    {
        state->player = alloc_entity(state->free);
        Entity *player = &state->entities[state->player];
        player->poly.points[0] = vec(PLAYER_PROP * PLAYER_LENGTH, 0.0);
        player->poly.points[1] = vec(0.0, 0.5 * PLAYER_WIDTH);
        player->poly.points[2] = vec(-(1 - PLAYER_PROP) * PLAYER_LENGTH, 0.0);
        player->poly.points[3] = vec(0.0, -0.5 * PLAYER_WIDTH);
        player->poly.n = 4;
        player->color = BLACK;
        player->cent = poly_centroid(&player->poly);
        entity_translate(player, vec_mul(-1.0, player->cent));
        player->v = vec(0.0, 0.0);
        player->a = vec(0.0, 0.0);
        player->theta = 0.0;
        player->omega = 0.0;
        player->health = 2;
    }

    // Spawn asteroids
    for (size_t i = 0; i < 5; i++) {
        spawn_asteroid(state);
    }

    // Initialize input state
    {
        state->input.status = PLAYING;
        state->input.quiting = false;
        state->input.restarting = false;
        state->input.thrusting = false;
        state->input.turning_clockwise = false;
        state->input.turning_counterclockwise = false;
        state->input.shooting = false;
    }

}


void update(GameState *state, double dt)
{
    if (state->input.restarting) {
        init_game(state);
        return;
    }

    // Update asteroids
    for (size_t i = 0; i < state->asteroids.length; i++) {
        Entity *entity = &state->entities[state->asteroids.idxs[i]];
        teleport(entity);
        entity_tick(entity, dt);
    }

    // Update bullets
    for (size_t i = 0; i < state->bullets.length; i++) {
        EntityIndex idx = state->bullets.idxs[i];
        Entity *bullet = &state->entities[idx];
        entity_tick(bullet, dt);

        Vector2 min = poly_min(&bullet->poly);
        Vector2 max = poly_max(&bullet->poly);
        if ((max.x < MIN.x && bullet->v.x < 0.0) ||
            (max.y < MIN.y && bullet->v.y < 0.0) ||
            (min.x > MAX.x && bullet->v.x > 0.0) ||
            (min.y > MAX.y && bullet->v.y > 0.0))
        {
            free_entity(state->free, idx);
            remove_index(&state->bullets, i);
            i--;
        }
    }

    if (state->input.status == PLAYING) {
        Entity *player = &state->entities[state->player];

        // Update player
        {
            player->a = vec_mul(-DRAG, player->v);
            teleport(player);
            if (state->input.thrusting) {
                Vector2 dir = vec(cos(player->theta), sin(player->theta));
                player->a = vec_add(player->a, vec_mul(THRUST, dir));
            }
            if (state->input.turning_clockwise && state->input.turning_counterclockwise) {
                player->omega = 0.0;
            } else if (state->input.turning_clockwise) {
                player->omega = -PLAYER_OMEGA;
            } else if (state->input.turning_counterclockwise) {
                player->omega = PLAYER_OMEGA;
            } else {
                player->omega = 0.0;
            }
            entity_tick(player, dt);
        }

        // Spawn bullets
        if (state->input.shooting) {
            EntityIndex idx = alloc_entity(state->free);
            push(&state->bullets, idx);
            Entity *bullet = &state->entities[idx];
            double theta = 0.0;
            double step = 2.0 * M_PI / BULLET_POINTS;
            Vector2 v = vec(0.0, BULLET_RAD);
            for (size_t i = 0; i < BULLET_POINTS; i++) {
                bullet->poly.points[i] = vec_rotate(theta, v);
                theta += step;
            }
            bullet->poly.n = BULLET_POINTS;
            bullet->color = RED;
            bullet->cent = poly_centroid(&bullet->poly);
            Vector2 dir = vec(cos(player->theta), sin(player->theta));
            entity_translate(bullet, vec_add(player->cent, vec_mul(PLAYER_LENGTH / 2.0, dir)));
            bullet->v = vec_mul(BULLET_VEL, dir);
            bullet->a = vec(0.0, 0.0);
            bullet->theta = 0.0;
            bullet->omega = 0.0;
            state->input.shooting = false;
        }

        // Find player/asteroid collisions
        {
            Polygon *player_poly = &state->entities[state->player].poly;

            for (size_t i = 0; i < state->asteroids.length; i++) {

                EntityIndex idx = state->asteroids.idxs[i];
                Polygon *asteroid_poly = &state->entities[idx].poly;

                if (find_collision(player_poly, asteroid_poly)) {
                    state->input.status = OVER;
                    free_entity(state->free, idx);
                    remove_index(&state->asteroids, i);
                    i--;
                }
            }
        }
    }

    // Find bullet/asteroid collisions
    for (size_t i = 0; i < state->asteroids.length; i++) {
        for (size_t j = 0; j < state->bullets.length; j++) {
            EntityIndex asteroid_idx = state->asteroids.idxs[i];
            EntityIndex bullet_idx = state->bullets.idxs[j];
            Entity *asteroid = &state->entities[asteroid_idx];
            Polygon *asteroid_poly = &state->entities[asteroid_idx].poly;
            Polygon *bullet_poly = &state->entities[bullet_idx].poly;

            if (find_collision(asteroid_poly, bullet_poly)) {
                asteroid->health -= 1;
                if (asteroid->health == 0) {
                    spawn_asteroid(state);
                } else {
                    spawn_asteroid_with_info(
                        state,
                        ASTEROID_RAD,
                        asteroid->color,
                        vec_add(asteroid->cent, vec(ASTEROID_RAD, 0.0)),
                        vec_mul(ASTEROID_VEL, rand_dir()),
                        1);
                    spawn_asteroid_with_info(
                        state,
                        ASTEROID_RAD,
                        asteroid->color,
                        vec_sub(asteroid->cent, vec(ASTEROID_RAD, 0.0)),
                        vec_mul(ASTEROID_VEL, rand_dir()),
                        1);
                }
                free_entity(state->free, bullet_idx);
                remove_index(&state->bullets, j);
                j--;
                free_entity(state->free, asteroid_idx);
                remove_index(&state->asteroids, i);
                i--;
                break;
            }
        }
    }
}

void render(const GameState *state)
{
    sdl_clear();

    // Render asteroids
    for (size_t i = 0; i < state->asteroids.length; i++) {
        const Entity *asteroid = &state->entities[state->asteroids.idxs[i]];
        sdl_draw_polygon(&asteroid->poly, asteroid->color);
    }

    // Render bullets
    for (size_t i = 0; i < state->bullets.length; i++) {
        const Entity *bullet = &state->entities[state->bullets.idxs[i]];
        sdl_draw_polygon(&bullet->poly, bullet->color);
    }

    // Render player
    if (state->input.status == PLAYING) {
        const Entity *player = &state->entities[state->player];
        sdl_draw_polygon(&player->poly, player->color);
    }

    sdl_show();
}

void on_key(char key, KeyEventType type, double held_time, InputState *input)
{
    switch(input->status) {
        case START:
        {
            // TODO
        } break;

        case PLAYING:
        {
            switch(key) {
                case UP_ARROW:
                {
                    if (type == KEY_PRESSED) {
                        input->thrusting = true;
                    } else {
                        input->thrusting = false;
                    }
                } break;

                case LEFT_ARROW:
                {
                    if (type == KEY_PRESSED) {
                        input->turning_counterclockwise = true;
                    } else {
                        input->turning_counterclockwise = false;
                    }
                } break;

                case RIGHT_ARROW:
                {
                    if (type == KEY_PRESSED) {
                        input->turning_clockwise = true;
                    } else {
                        input->turning_clockwise = false;
                    }
                } break;

                case ' ':
                {
                    if (type == KEY_PRESSED && held_time == 0.0) {
                        input->shooting = true;
                    }
                } break;

                default:
                {
                } break;
            }
        } break;

        case OVER:
        {
            switch(key) {
                case ENTER:
                {
                    if (type == KEY_PRESSED && held_time == 0.0) {
                        input->restarting = true;
                    }
                } break;

                default:
                {
                } break;
            }
        } break;
    }
}

int main(void)
{
    sdl_init();
    sdl_on_key((KeyHandler) on_key);
    static GameState state;
    init_game(&state);
    double t = 0.0;
    size_t frames = 0;

    while (sdl_running(&state.input)) {
        double dt = time_since_last_tick();
        t += dt;
        frames++;
        update(&state, dt);
        render(&state);
    }
    printf("%f fps\n", (double) frames / t);

    sdl_quit();
}
