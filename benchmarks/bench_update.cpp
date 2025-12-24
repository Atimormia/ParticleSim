#include "particlesim/particle_system.hpp"
#include "particlesim/spatial_partitioning.hpp"
#include "core/vector.hpp"
#include "benchmark/benchmark.h"
#include <random>

#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#else
#define ZoneScoped
#endif

using namespace particlesim;

template <typename Layout>
void populate_system(ParticleSystem<Layout> &ps, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        Particle p{};
        p.velocity.x = float(i % 100) * 0.01f;
        p.velocity.y = float(i % 50) * 0.01f;
        p.lifetime = 5.0f;
        ps.addParticle(p);
    }
}

template <typename Layout>
static void BM_Update(benchmark::State &state)
{
    ParticleSystem<Layout> ps(state.range(0));
    populate_system(ps, state.range(0));

    for (auto _ : state)
    {
        ZoneScoped;
        ps.update(0.016f, true); // simulate 1 frame (~16ms)
        benchmark::ClobberMemory();
    }
}

BENCHMARK_TEMPLATE(BM_Update, ParticleSystemDataAoS)
    ->Name("BM_Update_AoS")
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(50000);

BENCHMARK_TEMPLATE(BM_Update, ParticleSystemDataSoA)
    ->Name("BM_Update_SoA")
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(50000);

BENCHMARK_TEMPLATE(BM_Update, ParticleSystemDataAllocated)
    ->Name("BM_Update_Allocated")
    ->Arg(1000)
    ->Arg(10000)
    ->Arg(50000);    

std::vector<core::Vector2D> generateParticles(size_t N, const WorldBounds &bounds)
{
    std::vector<core::Vector2D> particles(N);

    std::mt19937 rng(12345);
    std::uniform_real_distribution<float> xdist(bounds.minX, bounds.maxX);
    std::uniform_real_distribution<float> ydist(bounds.minY, bounds.maxY);

    for (size_t i = 0; i < N; ++i)
    {
        particles[i] = {xdist(rng), ydist(rng)};
    }
    return particles;
}

template <typename T, float S = 1.f> //ISpatialPartition
static void BM_UniformGridBuild(benchmark::State &state)
{
    size_t N = state.range(0);

    PartitioningConfig cfg;
    cfg.cellSize = S;
    cfg.world = {0, 0, 1000, 1000};
    cfg.neighborReserve = 64;

    T grid(cfg);

    auto particles = generateParticles(N, cfg.world);
    grid.setPositions(particles);

    for (auto _ : state)
    {
        grid.build();
        benchmark::ClobberMemory();
    }

    state.SetItemsProcessed(N * state.iterations());
}

template <typename T, float S = 1.f> //ISpatialPartition
static void BM_UniformGridQuery(benchmark::State &state)
{
    size_t N = state.range(0);

    PartitioningConfig cfg;
    cfg.cellSize = S;
    cfg.world = {0, 0, 1000, 1000};
    cfg.neighborReserve = 64;

    T grid(cfg);

    auto particles = generateParticles(N, cfg.world);
    grid.setPositions(particles);
    grid.build();

    for (auto _ : state)
    {
        for (size_t i = 0; i < N; ++i)
        {
            benchmark::DoNotOptimize(grid.queryNeighborhood(static_cast<uint32_t>(i)));
        }
    }

    state.SetItemsProcessed(N * state.iterations());
}

BENCHMARK(BM_UniformGridBuild<UniformGrid>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridQuery<UniformGrid>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridQuery<UniformGrid, 0.5f>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridQuery<UniformGrid, 2.f>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridBuild<NoPartition>)->Arg(1000)->Arg(10000)->Arg(20000);
BENCHMARK(BM_UniformGridQuery<NoPartition>)->Arg(1000)->Arg(10000)->Arg(20000);

BENCHMARK_MAIN();
