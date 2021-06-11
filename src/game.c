#include "base.h"
#include "vector.h"
#include "color.h"
#include "const.h"
#include "collision.h"
#include "polygon.h"
#include "sdl_wrapper.h"

const usize PARTICLE_POINTS = 10;
const usize NUM_PARTICLES = 10;
const f64 PARTICLE_RAD = 1.0;
const f64 PARTICLE_VEL = 50.0;
const f64 MIN_GREY = 0.25;
const f64 MAX_GREY = 0.75;

const f64 PLAYER_LENGTH = 80.0;
const f64 PLAYER_WIDTH = 40.0;
const f64 PLAYER_PROP = 0.75;
const f64 THRUST = 750.0;
const f64 DRAG = 1.0;
const f64 PLAYER_OMEGA = 1.25 * M_PI;

const usize BULLET_POINTS = 10;
const f64 BULLET_RAD = 5.0;
const f64 BULLET_VEL = 600.0;

const f64 BIG_ASTEROID_RAD = 60.0;
const f64 ASTEROID_RAD = 30.0;
const f64 ASTEROID_VEL = 150.0;
const usize INIT_NUM_ASTEROIDS = 5;
const usize MAX_NUM_ASTEROIDS = 20;

const Vector2 MAX = {
    .x = WIDTH / 2.0,
    .y = HEIGHT / 2.0,
};
const Vector2 MIN = {
    .x = -WIDTH / 2.0,
    .y = -HEIGHT / 2.0,
};
const Color BLACK = { .r = 0.0, .g = 0.0, .b = 0.0, .a = 1.0 };
const Color RED = { .r = 1.0, .g = 0.0, .b = 0.0, .a = 1.0 };

f64 rand_f64(f64 min, f64 max)
{
    return (max - min) * (f64) rand() / (f64) RAND_MAX + min;
}

Vector2 rand_dir(void)
{
    f64 d = rand_f64(-1.0, 1.0);
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
    f64 theta;
    f64 omega;
    u8 health;
} Entity;

typedef usize EntityIndex;

typedef struct {
    EntityIndex idxs[MAX_ENTITIES];
    usize length;
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
    usize num_asteroids;
} GameState;

void push(EntityIndexArray *arr, EntityIndex idx)
{
    assert(arr->length < MAX_ENTITIES);
    arr->idxs[arr->length] = idx;
    arr->length += 1;
}

void remove_index(EntityIndexArray *arr, usize idx)
{
    assert(idx < arr->length);
    assert(arr->length > 0);
    for (usize i = idx; i < arr->length-1; i++) {
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
    for (usize i = 0; i < MAX_ENTITIES; i++) {
        if (free[i]) {
            free[i] = false;
            return i;
        }
    }
    // TODO: Return an invalid EntityIndex when running out of memory
    // maybe use make EntityIndex an i8 and return -1
    // this would require setting MAX_ENTITIES to be 100
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

void entity_rotate(Entity *entity, f64 theta)
{
    poly_rotate(&entity->poly, theta, entity->cent);
    entity->theta += theta;
}

void entity_tick(Entity *entity, f64 dt)
{
    entity->v = vec_add(entity->v, vec_mul(dt, entity->a));
    entity_translate(entity, vec_mul(dt, entity->v));
    entity_rotate(entity, dt * entity->omega);
}

void spawn_asteroid_with_info(
    GameState *state,
    f64 r,
    Color color,
    Vector2 cent,
    Vector2 v,
    u8 health)
{
    EntityIndex idx = alloc_entity(state->free);
    push(&state->asteroids, idx);
    state->num_asteroids += 1;
    Entity *entity = &state->entities[idx];
    {
        f64 theta = 0.0;
        f64 steps[ASTEROID_POINTS];
        f64 sum = 0.0;
        for (usize i = 0; i < ASTEROID_POINTS; i++) {
            steps[i] = rand_f64(0.0, 1.0);
            sum += steps[i];
        }
        Vector2 v = vec(0.0, r);
        for (usize i = 0; i < ASTEROID_POINTS; i++) {
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
}

void spawn_asteroid(GameState *state)
{
    const f64 i = rand_f64(MIN_GREY, MAX_GREY);
    Color c = {.r = i, .g = i, .b = i, .a = 1.0 };
    f64 r;
    u8 health;
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
            cent = vec(MIN.x - r, rand_f64(MIN.y, MAX.y));
        } break;
        case 1:
        {
            cent = vec(MAX.x + r, rand_f64(MIN.y, MAX.y));
        } break;
        case 2:
        {
            cent = vec(rand_f64(MIN.x, MAX.x), MIN.y - r);
        } break;
        case 3:
        {
            cent = vec(rand_f64(MIN.x, MAX.x), MAX.y + r);
        } break;
    }
    spawn_asteroid_with_info(
            state, r, c, cent, vec_mul(ASTEROID_VEL, rand_dir()), health);
}

void spawn_particles(
    GameState *state,
    usize n,
    f64 r,
    Color color,
    Vector2 cent)
{
    for (usize i = 0; i < n; i++) {
        EntityIndex idx = alloc_entity(state->free);
        push(&state->particles, idx);
        Entity *particle = &state->entities[idx];
        f64 theta = 0.0;
        f64 step = 2.0 * M_PI / PARTICLE_POINTS;
        Vector2 v = vec(0.0, PARTICLE_RAD);
        for (usize i = 0; i < PARTICLE_POINTS; i++) {
            particle->poly.points[i] = vec_rotate(theta, v);
            theta += step;
        }
        particle->poly.n = PARTICLE_POINTS;
        particle->color = color;
        particle->cent = poly_centroid(&particle->poly);
        entity_translate(particle,
                vec_add(cent, vec_mul(rand_f64(0.0, 1.0) * r, rand_dir())));
        particle->v = vec_mul(rand_f64(0.0, 1.0) * PARTICLE_VEL, rand_dir());
        particle->a = vec(0.0, 0.0);
        particle->theta = 0.0;
        particle->omega = 0.0;
    }
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
    sdl_play_start();

    // Free all existing entities
    for (usize i = 0; i < MAX_ENTITIES; i++) {
        free_entity(state->free, i);
    }
    clear(&state->asteroids);
    clear(&state->bullets);
    clear(&state->particles);
    state->num_asteroids = 0;

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
    for (usize i = 0; i < INIT_NUM_ASTEROIDS; i++) {
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


void update(GameState *state, f64 dt)
{
    if (state->input.restarting) {
        init_game(state);
        return;
    }

    // Update particles
    for (usize i = 0; i < state->particles.length; i++) {
        EntityIndex idx = state->particles.idxs[i];
        Entity *particle = &state->entities[idx];
        particle->color.a -= dt;
        if (particle->color.a < 0.0) {
            free_entity(state->free, idx);
            remove_index(&state->particles, i);
            i--;
        } else {
            entity_tick(particle, dt);
        }
    }

    // Update asteroids
    for (usize i = 0; i < state->asteroids.length; i++) {
        Entity *entity = &state->entities[state->asteroids.idxs[i]];
        teleport(entity);
        entity_tick(entity, dt);
    }

    // Update bullets
    for (usize i = 0; i < state->bullets.length; i++) {
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
            sdl_play_shoot();
            EntityIndex idx = alloc_entity(state->free);
            push(&state->bullets, idx);
            Entity *bullet = &state->entities[idx];
            f64 theta = 0.0;
            f64 step = 2.0 * M_PI / BULLET_POINTS;
            Vector2 v = vec(0.0, BULLET_RAD);
            for (usize i = 0; i < BULLET_POINTS; i++) {
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
            for (usize i = 0; i < state->asteroids.length; i++) {

                EntityIndex idx = state->asteroids.idxs[i];
                Entity *asteroid = &state->entities[idx];

                if (find_collision(&player->poly, &asteroid->poly)) {
                    sdl_play_hit();
                    sdl_play_game_over();
                    state->num_asteroids -= 1;
                    spawn_particles(
                        state, NUM_PARTICLES, PLAYER_LENGTH, BLACK, player->cent);
                    spawn_particles(
                        state, NUM_PARTICLES, ASTEROID_RAD, asteroid->color, asteroid->cent);
                    state->input.status = OVER;
                    free_entity(state->free, idx);
                    remove_index(&state->asteroids, i);
                    i--;
                }
            }
        }
    }

    // Find bullet/asteroid collisions
    for (usize i = 0; i < state->asteroids.length; i++) {
        for (usize j = 0; j < state->bullets.length; j++) {
            EntityIndex asteroid_idx = state->asteroids.idxs[i];
            EntityIndex bullet_idx = state->bullets.idxs[j];
            Entity *asteroid = &state->entities[asteroid_idx];
            Polygon *asteroid_poly = &state->entities[asteroid_idx].poly;
            Polygon *bullet_poly = &state->entities[bullet_idx].poly;

            if (find_collision(asteroid_poly, bullet_poly)) {
                sdl_play_hit();
                asteroid->health -= 1;
                state->num_asteroids -= 1;
                if (asteroid->health == 0) {
                    spawn_particles(
                        state, NUM_PARTICLES, ASTEROID_RAD, asteroid->color, asteroid->cent);
                    if (state->num_asteroids < MAX_NUM_ASTEROIDS) {
                        spawn_asteroid(state);
                    }
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

    // Render particles
    for (usize i = 0; i < state->particles.length; i++) {
        const Entity *particle = &state->entities[state->particles.idxs[i]];
        sdl_draw_polygon(&particle->poly, particle->color);
    }

    // Render asteroids
    for (usize i = 0; i < state->asteroids.length; i++) {
        const Entity *asteroid = &state->entities[state->asteroids.idxs[i]];
        sdl_draw_polygon(&asteroid->poly, asteroid->color);
    }

    // Render bullets
    for (usize i = 0; i < state->bullets.length; i++) {
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

void on_key(u8 key, KeyEventType type, f64 held_time, InputState *input)
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
                    if (type == KEY_PRESSED && held_time == 0.0) {
                        input->thrusting = true;
                        sdl_play_thrust();
                    }
                    if (type == KEY_RELEASED) {
                        input->thrusting = false;
                        sdl_stop_thrust();
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
    f64 t = 0.0;
    usize frames = 0;

    while (sdl_running(&state.input)) {
        f64 dt = time_since_last_tick();
        t += dt;
        frames++;
        update(&state, dt);
        render(&state);
    }
    printf("%f fps\n", (f64) frames / t);

    sdl_quit();
}
