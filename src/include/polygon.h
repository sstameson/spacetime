#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "vector.h"
#include "const.h"

typedef struct {
    Vector2 points[MAX_POINTS];
    size_t n;
} Polygon;

void poly_translate(Polygon *poly, Vector2 t);

void poly_rotate(Polygon *poly, double theta, Vector2 v);

Vector2 poly_min(Polygon *poly);

Vector2 poly_max(Polygon *poly);

double poly_area(Polygon *poly);

Vector2 poly_centroid(Polygon *poly);

#endif
