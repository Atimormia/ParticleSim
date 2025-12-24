#pragma once
#include <vector>
#include <string>
#include <span>
#include <memory>

#include "core/soa_container.hpp"
#include "core/vector.hpp"
#include "core/free_list.hpp"
#include "particle.hpp"
#include "spatial_partitioning.hpp"

namespace particlesim
{
    template <typename T>
    concept ParticleDataContainer = requires(T layout, float dt, const Particle &p, bool compact) {
        T{size_t{}};
        { layout.update(dt, compact) } -> same_as<void>;
        { layout.size() } -> std::same_as<size_t>;
        { layout.add(p) } -> std::same_as<size_t>;
        { layout.positions() } -> same_as<span<const core::Vector2D>>;
        #ifdef ENABLE_TEST_METHODS
        // for testing purposes
        { layout.get() } -> std::same_as<std::vector<Particle>>;
        #endif
    };

    template <ParticleDataContainer Layout>
    class ParticleSystem
    {
    public:
        ParticleSystem(size_t capacity = 100000, std::unique_ptr<ISpatialPartition> p = nullptr) : data(capacity), partition(std::move(p)) {}

        void setPartition(std::unique_ptr<ISpatialPartition> p) { partition = std::move(p); }

        size_t addParticle(const Particle &p) { return data.add(p); }

        void update(float dt, bool compact = false)
        {
            data.update(dt, compact);
            if (partition)
            {
                partition->setPositions(data.positions());
                partition->build();
            }
        }

        size_t size() const { return data.size(); }

        #ifdef ENABLE_TEST_METHODS
        // for testing purposes
        std::vector<Particle> get() { return data.get(); }
        #endif

    private:
        Layout data;
        std::unique_ptr<ISpatialPartition> partition = nullptr;
    };

    class ParticleSystemDataAoS
    {
    public:
        ParticleSystemDataAoS(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;

        span<const core::Vector2D> positions();
        #ifdef ENABLE_TEST_METHODS
        // for testing purposes
        std::vector<Particle> get() { return particles; }
        #endif

    private:
        std::vector<Particle> particles;
    };

    class ParticleSystemDataSoA
    {
    public:
        ParticleSystemDataSoA(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;

        span<const core::Vector2D> positions();
        #ifdef ENABLE_TEST_METHODS
        // for testing purposes
        std::vector<Particle> get();
        #endif

    private:
        ParticleSoA particles;

        void compactDead();
        const auto fields()
        {
            return tie(
                particles.field<Position>(),
                particles.field<Velocity>(),
                particles.field<Acceleration>(),
                particles.field<Lifetime>(),
                particles.field<Alive>());
        }
    };

    class ParticleSystemDataAllocated
    {
    public:
        explicit ParticleSystemDataAllocated(size_t capacity) : pool_(capacity)
        {
            activeIndices_.reserve(capacity);
        }

        size_t add(const Particle &p);

        void update(float dt);

        size_t size() const
        {
            return activeIndices_.size();
        }

        span<const core::Vector2D> positions();

        #ifdef ENABLE_TEST_METHODS
        std::vector<Particle> get() const;
        #endif

    private:
        core::FreeListPool<Particle> pool_;
        std::vector<size_t> activeIndices_;
    };
}
