#pragma once
#include <vector>
#include <string>
#include <span>
#include <memory>

#include "core/soa_container.hpp"
#include "core/vector.hpp"
#include "core/free_list.hpp"
#include "core/memory_arena.hpp"
#include "core/parallel_scheduler.hpp"
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
        // for testing purposes
        { layout.get() } -> std::same_as<std::vector<Particle>>;
    };

    template <typename T>
    concept ParticleDataContainerParallel = ParticleDataContainer<T> && requires(T layout, float dt, IScheduler& scheduler, bool compact) {
        { layout.updateParallel(dt, scheduler, compact) } -> std::same_as<void>;
    };

    template <ParticleDataContainer Layout>
    class ParticleSystem
    {
    public:
        ParticleSystem(size_t capacity = 100000, std::unique_ptr<ISpatialPartition> p = nullptr)
            : data(capacity), partition(std::move(p)), arena_(estimateArenaSize(capacity)), scheduler_() {}

        void setPartition(std::unique_ptr<ISpatialPartition> p) { partition = std::move(p); }

        size_t addParticle(const Particle &p) { return data.add(p); }

        void update(float dt, bool compact = false)
        {
            PartitionData partitionData;
            partitionData.positions = data.positions();
            partitionData.arena = &arena_;
            if constexpr (ParticleDataContainerParallel<Layout>)
            {
                partitionData.scheduler = &scheduler_;
                data.updateParallel(dt, scheduler_, compact);
            }
            else
                data.update(dt, compact);
            if (partition)
            {
                partition->clear();
                partition->setData(partitionData);
                partition->build();
            }
        }

        size_t size() const { return data.size(); }

        // for testing purposes
        std::vector<Particle> get() { return data.get(); }

    private:
        Layout data;
        std::unique_ptr<ISpatialPartition> partition = nullptr;
        core::FrameArena arena_;
        core::ThreadPoolScheduler scheduler_;

        size_t estimateArenaSize(size_t particleCount)
        {
            return (particleCount * 16) + (particleCount * sizeof(uint32_t) * 8) + (64 * 1024);
        }
    };

    class ParticleSystemDataAoS
    {
    public:
        ParticleSystemDataAoS(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;

        span<const core::Vector2D> positions();
        // for testing purposes
        std::vector<Particle> get();

    private:
        std::vector<Particle> particles;
        std::vector<Vector2D> positionsCache_;
    };

    class ParticleSystemDataSoA
    {
    public:
        ParticleSystemDataSoA(size_t capacity = 100000);

        void update(float dt, bool compact = false);
        size_t add(const Particle &p);
        size_t size() const;

        span<const core::Vector2D> positions();
        // for testing purposes
        std::vector<Particle> get();

    private:
        ParticleSoA particles;
        std::vector<Vector2D> positionsCache_;

    protected:
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

    class ParticleSystemDataSoAParallelized : public ParticleSystemDataSoA
    {
    public:
        void updateParallel(float dt, core::IScheduler &scheduler, bool compact = false);
    };

    class ParticleSystemDataAllocated
    {
    public:
        explicit ParticleSystemDataAllocated(size_t capacity) : pool_(capacity)
        {
            activeIndices_.reserve(capacity);
        }

        size_t add(const Particle &p);
        void update(float dt, bool compact = false);
        size_t size() const
        {
            return activeIndices_.size();
        }

        span<const core::Vector2D> positions();
        // for testing purposes
        std::vector<Particle> get();

    private:
        core::FreeListPool<Particle> pool_;
        std::vector<size_t> activeIndices_;
        std::vector<Vector2D> positionsCache_;
    };
}
