#include <stdlib.h>
#include <math.h>

#include "vector.h"
#include "polygon.h"

void poly_translate(Polygon *poly, Vector2 t)
{
    for (size_t i = 0; i < poly->n; i++) {
        poly->points[i] = vec_add(poly->points[i], t);
    }
}

void poly_rotate(Polygon *poly, double theta, Vector2 v)
{
    for (size_t i = 0; i < poly->n; i++) {
        poly->points[i] = vec_add(vec_rotate(theta, vec_sub(poly->points[i], v)), v);
    }
}

Vector2 poly_min(Polygon *poly)
{
    Vector2 min = vec(INFINITY, INFINITY);
    for (size_t i = 0; i < poly->n; i++) {
        if (poly->points[i].x < min.x) min.x = poly->points[i].x;
        if (poly->points[i].y < min.y) min.y = poly->points[i].y;
    }
    return min;
}

Vector2 poly_max(Polygon *poly)
{
    Vector2 max = vec(-INFINITY, -INFINITY);
    for (size_t i = 0; i < poly->n; i++) {
        if (poly->points[i].x > max.x) max.x = poly->points[i].x;
        if (poly->points[i].y > max.y) max.y = poly->points[i].y;
    }
    return max;
}

double poly_signed_area(Polygon *poly)
{
    double area = 0;
    for (size_t i = 0; i < poly->n; i++) {
        area += vec_cross(poly->points[i], poly->points[(i+1) % poly->n]);
    }
    return 1.0 / 2.0 * area;
}

double poly_area(Polygon *poly)
{
    return fabs(poly_signed_area(poly));
}

Vector2 poly_centroid(Polygon *poly)
{
    Vector2 c = vec(0.0, 0.0);
    for (size_t i = 0; i < poly->n; i++) {
        Vector2 v1 = poly->points[i];
        Vector2 v2 = poly->points[(i+1) % poly->n];
        c = vec_add(c, vec_mul(vec_cross(v1, v2), vec_add(v1, v2)));
    }
    return vec_mul(1.0 / (6.0 * poly_signed_area(poly)), c);
}

