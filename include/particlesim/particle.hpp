#pragma once
#include "math/vector.hpp"
#include <type_traits>
#include <string>
namespace particlesim
{
    struct Particle
    {
        math::Vector2D position{0.0f, 0.0f};
        math::Vector2D velocity{0.0f, 0.0f};
        math::Vector2D acceleration{0.0f, 0.0f};
        float lifetime = 0.0f;
        bool alive = true;

        void reset();
        void kill();
        void update(float dt);

        std::string tostring() const; 
    };

    static_assert(std::is_trivially_copyable_v<Particle>);
} // namespace particlesim