#include <random>
#include <vector>
#include "core/vector.hpp"
#include "particlesim/spatial_partitioning.hpp"
#include "particlesim/particle.hpp"
#include "benchmark/benchmark.h"

using namespace particlesim;

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

template <typename T>
struct PartitioningBenchmarkData
{
    std::vector<Vector2D> particles;
    core::FrameArena arena;
    T grid;

    PartitioningBenchmarkData(size_t range, float cellSize)
        : particles(),
          grid(makeConfig(cellSize)),
          arena((range * sizeof(uint32_t) * 32) + (64 * 1024))
    {
        particles = generateParticles(range, grid.config.world);
        grid.setData({particles, std::move(arena)});
    }

    static PartitioningConfig makeConfig(float cellSize)
    {
        PartitioningConfig cfg;
        cfg.cellSize = cellSize;
        cfg.world = {0.f, 0.f, 1000.f, 1000.f};
        cfg.neighborReserve = 64;
        return cfg;
    }
};

template <typename T, float S = 1.f> // ISpatialPartition
static void BM_UniformGridBuild(benchmark::State &state)
{
    size_t N = state.range(0);
    auto data = PartitioningBenchmarkData<T>(N, S);

    for (auto _ : state)
    {
        data.arena.reset();
        data.grid.build();
    }

    state.SetItemsProcessed(N * state.iterations());
}

template <typename T, float S = 1.f> // ISpatialPartition
static void BM_UniformGridQuery(benchmark::State &state)
{
    size_t N = state.range(0);
    auto grid = PartitioningBenchmarkData<T>(N, S);
    grid.grid.build();

    for (auto _ : state)
    {
        for (size_t i = 0; i < N; ++i)
        {
            grid.arena.reset();
            benchmark::DoNotOptimize(grid.grid.queryNeighborhood(static_cast<uint32_t>(i)));            
        }
    }

    state.SetItemsProcessed(N * state.iterations());
}

BENCHMARK(BM_UniformGridQuery<UniformGridAllocated>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridBuild<UniformGrid>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridQuery<UniformGrid>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridQuery<UniformGrid, 0.5f>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridQuery<UniformGrid, 2.f>)->Arg(1000)->Arg(10000)->Arg(50000)->Arg(100000);
BENCHMARK(BM_UniformGridBuild<NoPartition>)->Arg(1000)->Arg(10000)->Arg(20000);
BENCHMARK(BM_UniformGridQuery<NoPartition>)->Arg(1000)->Arg(10000)->Arg(20000);