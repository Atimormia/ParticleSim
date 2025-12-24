#pragma once
#include <type_traits>
#include <string>
#include <cstdint>

#include "core/vector.hpp"
#include "core/soa_container.hpp"
namespace particlesim
{
    using namespace core;
    
    struct Position {};
    struct Velocity {};
    struct Acceleration {};
    struct Lifetime {};
    struct Alive {};
    struct Particle
    {
        Vector2D position{0.0f, 0.0f};
        Vector2D velocity{0.0f, 0.0f};
        Vector2D acceleration{0.0f, 0.0f};
        float lifetime = 0.0f;
        bool alive = true;

        void reset();
        void kill();
        void update(float dt);
    };

    static_assert(std::is_trivially_copyable_v<Particle>);
    using ParticleSoA = SoAContainer<
        SoAFieldVector2D<Position>,
        SoAFieldVector2D<Velocity>,
        SoAFieldVector2D<Acceleration>, 
        SoAFieldScalar<float, Lifetime>,     
        SoAFieldScalar<uint8_t, Alive>   
        >;

}