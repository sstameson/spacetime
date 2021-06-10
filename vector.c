#include <math.h>

#include "vector.h"

Vector2 vec(double x, double y)
{
    return (Vector2) { x, y };
}

Vector2 vec_mul(double a, Vector2 v)
{
    return vec(a * v.x, a * v.y);
}

Vector2 vec_add(Vector2 v1, Vector2 v2)
{
    return vec(v1.x + v2.x, v1.y + v2.y);
}

Vector2 vec_sub(Vector2 v1, Vector2 v2)
{
    return vec(v1.x - v2.x, v1.y - v2.y);
}

double vec_dot(Vector2 v1, Vector2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(Vector2 v1, Vector2 v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

Vector2 vec_proj(Vector2 v, Vector2 u)
{
    return vec_mul(vec_dot(v, u) / vec_dot(u, u), u);
}

Vector2 vec_rotate(double theta, Vector2 v)
{
    return vec(v.x * cos(theta) - v.y * sin(theta),
               v.x * sin(theta) + v.y * cos(theta));
}
