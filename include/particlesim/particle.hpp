#pragma once
#include <type_traits>
#include <string>
#include <cstdint>
#include <vector>

#include "core/vector.hpp"
#include "core/soa_container.hpp"
#include "core/allocator.hpp"
namespace particlesim
{
    struct Position {};
    struct Velocity {};
    struct Acceleration {};
    struct Lifetime {};
    struct Alive {};
    struct Particle
    {
        core::Vector2D position{0.0f, 0.0f};
        core::Vector2D velocity{0.0f, 0.0f};
        core::Vector2D acceleration{0.0f, 0.0f};
        float lifetime = 0.0f;
        bool alive = true;

        void reset();
        void kill();
        void update(float dt);
    };

    static_assert(std::is_trivially_copyable_v<Particle>);
    using ParticleSoA = core::SoAContainer<
        core::SoAFieldVector2D<Position>,
        core::SoAFieldVector2D<Velocity>,
        core::SoAFieldVector2D<Acceleration>, 
        core::SoAFieldScalar<float, Lifetime>,     
        core::SoAFieldScalar<uint8_t, Alive>   
        >;

    using ParticlePoolAllocator = core::PoolAllocator<Particle>;
    using ParticleAoSVector = std::vector<Particle, ParticlePoolAllocator>;
}