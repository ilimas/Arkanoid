
#ifndef UTILS_H
#define UTILS_H

#include <SDL_rect.h>
#include <cmath>
#include <cstdint>
#include <ostream>

enum class BrickTouchSide : uint8_t
{
    NO_TOUCH,
    LEFT_EDGE,
    RIGHT_EDGE,
    UP_EDGE,
    DOWN_EDGE
};

enum class BoundsTouchSide : uint8_t
{
    NO_TOUCH,
    LEFT_BORDER,
    RIGHT_BORDER,
    UP_BORDER,
    DOWN_BORDER
};

struct Vec2
{
    double x, y;

    Vec2() : x(0.0), y(0.0) {}
    Vec2(double x_, double y_) : x(x_), y(y_) {}

    Vec2 operator-() const
    {
        return Vec2(-x, -y);
    }

    Vec2 operator+(Vec2 const &rhs) const
    {
        return Vec2(x + rhs.x, y + rhs.y);
    }

    Vec2 operator-(Vec2 const &rhs) const
    {
        return Vec2(x - rhs.x, y - rhs.y);
    }

    Vec2 operator*(double s) const
    {
        return Vec2(x * s, y * s);
    }

    Vec2 operator/(double s) const
    {
        return Vec2(x / s, y / s);
    }

    Vec2 &operator+=(Vec2 const &rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }
    Vec2 &operator-=(Vec2 const &rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }
    Vec2 &operator*=(double s)
    {
        x *= s;
        y *= s;
        return *this;
    }
    Vec2 &operator/=(double s)
    {
        x /= s;
        y /= s;
        return *this;
    }

    double dot(Vec2 const &rhs) const
    {
        return x * rhs.x + y * rhs.y;
    }

    double length() const
    {
        return std::sqrt(x * x + y * y);
    }

    Vec2 normalized() const
    {
        double len = length();
        return len > 0.0 ? (*this) / len : Vec2(0.0, 0.0);
    }

    friend std::ostream &operator<<(std::ostream &os, Vec2 const &v)
    {
        os << "(" << v.x << ", " << v.y << ")";
        return os;
    }
};

inline Vec2 operator*(double s, Vec2 const &v)
{
    return Vec2(v.x * s, v.y * s);
}

inline Vec2 reflect(const Vec2 &destination, const Vec2 &normal)
{
    float dotIN = destination.dot(normal);
    return destination - normal * (2 * dotIN);
}

inline Vec2 rotate(const Vec2 &v, double angleRad)
{
    double c = std::cos(angleRad);
    double s = std::sin(angleRad);
    return {
        v.x * c - v.y * s,
        v.x * s + v.y * c};
}

constexpr double map_range(double x, double in_min, double in_max, double out_min, double out_max)
{
    double normalized = (x - in_min) * 1.f / (in_max - in_min);
    double result = out_min + normalized * (out_max - out_min);
    return result;
}

inline Vec2 closestPoint(SDL_Rect block_rect, Vec2 ballPos)
{
    float x = std::max<float>(block_rect.x, std::min<float>(ballPos.x, block_rect.x + block_rect.w));
    float y = std::max<float>(block_rect.y, std::min<float>(ballPos.y, block_rect.y + block_rect.h));
    return Vec2(x, y);
}

inline Vec2 closestPointOnSegment(Vec2 A, Vec2 B, Vec2 P)
{
    Vec2 AB = B - A;
    float ab2 = AB.dot(AB);
    if (ab2 == 0.0f)
    {
        return A;
    }
    float t = (P - A).dot(AB) / ab2;
    if (t < 0.0f)
        t = 0.0f;
    else if (t > 1.0f)
        t = 1.0f;
    return A + AB * t;
}

#endif
