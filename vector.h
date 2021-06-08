#ifndef __VECTOR_H__
#define __VECTOR_H__

#include <math.h>

typedef struct {
    double x;
    double y;
} Vector2;

Vector2 vec(double x, double y) {
    return (Vector2) { x, y };
}

Vector2 vec_mul(double a, Vector2 v) {
    return vec(a * v.x, a * v.y);
}

Vector2 vec_add(Vector2 v1, Vector2 v2) {
    return vec(v1.x + v2.x, v1.y + v2.y);
}

Vector2 vec_rotate(double theta, Vector2 v) {
    return vec(v.x * cos(theta) - v.y * sin(theta),
               v.x * sin(theta) + v.y * cos(theta));
}

#endif
