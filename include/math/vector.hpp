#pragma once
#include <sstream>
namespace math
{
    struct Vector2D
    {
        float x = 0.f;
        float y = 0.f;

        constexpr Vector2D() {}
        constexpr Vector2D(float x_, float y_) : x(x_), y(y_) {}

        constexpr Vector2D operator+(const Vector2D &other) const
        {
            return {x + other.x, y + other.y};
        }

        constexpr Vector2D operator-(const Vector2D &other) const
        {
            return {x - other.x, y - other.y};
        }

        constexpr Vector2D operator*(float scalar) const
        {
            return {x * scalar, y * scalar};
        }

        constexpr Vector2D &operator+=(const Vector2D &other)
        {
            x += other.x;
            y += other.y;
            return *this;
        }

        constexpr Vector2D &operator-=(const Vector2D &other)
        {
            x -= other.x;
            y -= other.y;
            return *this;
        }

        constexpr Vector2D &operator*=(const Vector2D &other)
        {
            x *= other.x;
            y *= other.y;
            return *this;
        }

        std::string tostring() const
        {
            std::ostringstream ss;
            ss << "(" << x << "," << y << ")";
            return ss.str();
        }
    };
}
