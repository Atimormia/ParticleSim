#include <benchmark/benchmark.h>
#include "particlesim/particle_system.hpp"
#include "tracy/Tracy.hpp"

using namespace particlesim;

static void BM_Update(benchmark::State& state)
{
    ParticleSystem ps(state.range(0));

    for (size_t i = 0; i < ps.particles().size(); ++i) {
        Particle p{};
        p.velocity.x = float(i % 100) * 0.01f;
        p.lifetime = 5.0f;
        ps.add_particle(p);
    }

    for (auto _ : state) {
        ZoneScoped; // Tracy profiling zone
        ps.update(0.016f);
    }
}

BENCHMARK(BM_Update)->Arg(1000)->Arg(10000)->Arg(50000);

BENCHMARK_MAIN();
