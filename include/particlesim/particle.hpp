#pragma once
#include "math/vector.hpp"
#include <type_traits>
#include <string>
#include "soa_container.hpp"
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
    using ParticleSoA = SoAContainer<
        SoAFieldVector2D, // 0 - position
        SoAFieldVector2D, // 1 - velocity
        SoAFieldVector2D, // 2 - acceleration
        SoAFieldScalar<float>,        // 3 - lifetime
        SoAFieldScalar<uint8_t>          // 4 - alive flag
        >;

} // namespace particlesim