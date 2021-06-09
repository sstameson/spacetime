#ifndef _POLYGON_H_
#define _POLYGON_H_

#include "vector.h"

void translate_poly(Vector2 *poly, size_t n, Vector2 t)
{
    for (size_t i = 0; i < n; i++) {
        poly[i] = vec_add(poly[i], t);
    }
}

#endif
