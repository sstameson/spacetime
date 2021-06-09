#include <stdlib.h>
#include <math.h>

#include "vector.h"

void poly_translate(Vector2 *poly, size_t n, Vector2 t)
{
    for (size_t i = 0; i < n; i++) {
        poly[i] = vec_add(poly[i], t);
    }
}

Vector2 poly_min(Vector2 *poly, size_t n)
{
    Vector2 min = vec(INFINITY, INFINITY);
    for (size_t i = 0; i < n; i++) {
        if (poly[i].x < min.x) min.x = poly[i].x;
        if (poly[i].y < min.y) min.y = poly[i].y;
    }
    return min;
}

Vector2 poly_max(Vector2 *poly, size_t n)
{
    Vector2 max = vec(-INFINITY, -INFINITY);
    for (size_t i = 0; i < n; i++) {
        if (poly[i].x > max.x) max.x = poly[i].x;
        if (poly[i].y > max.y) max.y = poly[i].y;
    }
    return max;
}
