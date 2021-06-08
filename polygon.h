#ifndef __POLYGON_H__
#define __POLYGON_H__

#include "vector.h"

void translate_poly(Vector2 *poly, size_t n, Vector2 t)
{
    for (size_t i = 0; i < n; i++) {
        poly[i] = vec_add(poly[i], t);
    }
}

#endif
