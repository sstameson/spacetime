#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "base.h"
#include "vector.h"
#include "polygon.h"
#include "const.h"

typedef struct {
    Vector2 points[MAX_POINTS];
    usize n;
} Polygon;

void poly_translate(Polygon *poly, Vector2 t);

void poly_rotate(Polygon *poly, f64 theta, Vector2 v);

Vector2 poly_min(Polygon *poly);

Vector2 poly_max(Polygon *poly);

f64 poly_area(Polygon *poly);

Vector2 poly_centroid(Polygon *poly);

#endif
