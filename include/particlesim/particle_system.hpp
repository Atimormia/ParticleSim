#pragma once
#include <vector>
#include <string>
#include <span>
#include <memory>

#include "core/soa_container.hpp"
#include "core/vector.hpp"
#include "particle.hpp"
#include "spatial_partitioning.hpp"

namespace particlesim
{
    using namespace std;
    using namespace core;
    
    template <typename T>
    concept ParticleDataContainer = requires(T layout, float dt, const Particle &p) {
        { layout.update(dt) } -> same_as<void>;
        { layout.size() } -> integral;
        { layout.add(p) } -> integral;
        { layout.positions() } -> same_as<span<const Vector2D>>;
    };

    template <ParticleDataContainer Layout>
    class ParticleSystem
    {
    public:
        ParticleSystem(size_t capacity = 100000, unique_ptr<ISpatialPartition> p = nullptr) : data(capacity), partition(move(p)) {}

        void setPartition(unique_ptr<ISpatialPartition> p) { partition = move(p); }

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

        vector<Particle> get() { return data.get(); }

    private:
        Layout data;
        unique_ptr<ISpatialPartition> partition = nullptr;
    };

    class ParticleSystemDataAoS
    {
    public:
        ParticleSystemDataAoS(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;

        span<const Vector2D> positions();
        vector<Particle> get() { return particles; }

    private:
        vector<Particle> particles;
    };

    class ParticleSystemDataSoA
    {
    public:
        ParticleSystemDataSoA(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;
        
        span<const Vector2D> positions();
        vector<Particle> get();

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
                particles.field<Alive>()
            );
        }
    };
}
