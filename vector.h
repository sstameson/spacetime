#ifndef _VECTOR_H_
#define _VECTOR_H_

typedef struct {
    double x;
    double y;
} Vector2;

Vector2 vec(double x, double y);

Vector2 vec_mul(double a, Vector2 v);

Vector2 vec_add(Vector2 v1, Vector2 v2);

Vector2 vec_sub(Vector2 v1, Vector2 v2);

double vec_cross(Vector2 v1, Vector2 v2);

Vector2 vec_rotate(double theta, Vector2 v);

#endif
