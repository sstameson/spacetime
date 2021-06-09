#include <stdlib.h>
#include <math.h>

#include "vector.h"

void poly_translate(Vector2 *poly, size_t n, Vector2 t)
{
    for (size_t i = 0; i < n; i++) {
        poly[i] = vec_add(poly[i], t);
    }
}

void poly_rotate(Vector2 *poly, size_t n, double theta, Vector2 p)
{
    for (size_t i = 0; i < n; i++) {
        poly[i] = vec_add(vec_rotate(theta, vec_sub(poly[i], p)), p);
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

double poly_signed_area(Vector2 *poly, size_t n)
{
    double area = 0;
    for (size_t i = 0; i < n; i++) {
        area += vec_cross(poly[i], poly[(i+1) % n]);
    }
    return 1.0 / 2.0 * area;
}

double poly_area(Vector2 *poly, size_t n)
{
    return fabs(poly_signed_area(poly, n));
}

Vector2 poly_centroid(Vector2 *poly, size_t n)
{
    Vector2 c = vec(0.0, 0.0);
    for (size_t i = 0; i < n; i++) {
        Vector2 v1 = poly[i];
        Vector2 v2 = poly[(i+1) % n];
        c = vec_add(c, vec_mul(vec_cross(v1, v2), vec_add(v1, v2)));
    }
    return vec_mul(1.0 / (6.0 * poly_signed_area(poly, n)), c);
}

