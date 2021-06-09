#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "vector.h"

void poly_translate(Vector2 *poly, size_t n, Vector2 t);

void poly_rotate(Vector2 *poly, size_t n, double theta, Vector2 p);

Vector2 poly_min(Vector2 *poly, size_t n);

Vector2 poly_max(Vector2 *poly, size_t n);

double poly_area(Vector2 *poly, size_t n);

Vector2 poly_centroid(Vector2 *poly, size_t n);

#endif
