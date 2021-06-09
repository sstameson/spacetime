#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "vector.h"

void poly_translate(Vector2 *poly, size_t n, Vector2 t);

Vector2 poly_min(Vector2 *poly, size_t n);

Vector2 poly_max(Vector2 *poly, size_t n);

#endif
