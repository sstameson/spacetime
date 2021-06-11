#include "collision.h"
#include "vector.h"

typedef struct {
    f64 min;
    f64 max;
} Bounds;

/* Compute the min and max x-values of projecting poly onto u */
Bounds get_bounds(Polygon *poly, Vector2 u)
{
    f64 min = INFINITY;
    f64 max = -INFINITY;
    for (usize i = 0; i < poly->n; i++) {
        Vector2 proj = vec_proj(poly->points[i], u);
        if (proj.x < min) {
            min = proj.x;
        }
        if (proj.x > max) {
            max = proj.x;
        }
    }
    Bounds b = { .min = min, .max = max }; 
    return b;
}

/* Return true if projection of poly1 and poly2 onto u overlap */
bool axes_overlap(Polygon *poly1, Polygon *poly2, Vector2 u)
{
    Bounds b1 = get_bounds(poly1, u);
    Bounds b2 = get_bounds(poly2, u);
    return ((b1.max >= b2.min && b1.min <= b2.max) ||
            (b2.max >= b1.min && b2.min <= b1.max));
}

/* Iterate through projection vectors in poly */
bool find_collision_shape(Polygon *poly, Polygon *poly1, Polygon *poly2)
{
    for (usize i = 0; i < poly->n; i++) {
        Vector2 u = vec_rotate(
                M_PI / 2.0, vec_sub(poly->points[i], poly->points[(i+1) % poly->n]));
        if (!axes_overlap(poly1, poly2, u)) {
            return false;
        }
    }
    return true;
}

bool find_collision(Polygon *poly1, Polygon *poly2)
{
    return (find_collision_shape(poly1, poly1, poly2) &&
            find_collision_shape(poly2, poly1, poly2));
}
