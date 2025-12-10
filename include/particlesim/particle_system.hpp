#pragma once
#include <vector>
#include <string>
#include <span>

#include "particle.hpp"
#include "soa_container.hpp"
#include "spatial_partitioning.hpp"
#include "math/vector.hpp"

namespace particlesim
{
    template <typename T>
    concept ParticleDataContainer = requires(T layout, float dt, const Particle &p) {
        { layout.update(dt) } -> std::same_as<void>;
        { layout.size() } -> std::integral;
        { layout.add(p) } -> std::integral;
        { layout.positions() } -> std::same_as<std::span<const math::Vector2D>>;
    };

    template <ParticleDataContainer Layout>
    class ParticleSystem
    {
    public:
        ParticleSystem(size_t capacity = 100000, std::unique_ptr<ISpatialPartition> p = nullptr) : data(capacity), partition(std::move(p)) {}

        void setPartition(std::unique_ptr<ISpatialPartition> p) { partition = std::move(p); }

        size_t add_particle(const Particle &p) { return data.add(p); }

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

        std::vector<Particle> get() { return data.get(); }

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

        std::span<const math::Vector2D> positions();
        std::vector<Particle> get() { return particles; }

        std::string tostring() const;

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
        
        std::span<const math::Vector2D> positions();
        std::vector<Particle> get();

    private:
        ParticleSoA particles;

        void compact_dead();
    };

} // namespace particlesim
