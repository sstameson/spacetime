#ifndef _VECTOR_H_
#define _VECTOR_H_

#include "base.h"

typedef struct {
    f64 x;
    f64 y;
} Vector2;

Vector2 vec(f64 x, f64 y);

Vector2 vec_mul(f64 a, Vector2 v);

Vector2 vec_add(Vector2 v1, Vector2 v2);

Vector2 vec_sub(Vector2 v1, Vector2 v2);

f64 vec_cross(Vector2 v1, Vector2 v2);

f64 vec_dot(Vector2 v1, Vector2 v2);

Vector2 vec_proj(Vector2 v, Vector2 u);

Vector2 vec_rotate(f64 theta, Vector2 v);

#endif
