#include "vector.h"

Vector2 vec(f64 x, f64 y)
{
    return (Vector2) { x, y };
}

Vector2 vec_mul(f64 a, Vector2 v)
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

f64 vec_dot(Vector2 v1, Vector2 v2)
{
    return v1.x * v2.x + v1.y * v2.y;
}

f64 vec_cross(Vector2 v1, Vector2 v2)
{
    return v1.x * v2.y - v1.y * v2.x;
}

Vector2 vec_proj(Vector2 v, Vector2 u)
{
    return vec_mul(vec_dot(v, u) / vec_dot(u, u), u);
}

Vector2 vec_rotate(f64 theta, Vector2 v)
{
    return vec(v.x * cos(theta) - v.y * sin(theta),
               v.x * sin(theta) + v.y * cos(theta));
}
