#include "particlesim/particle_system.hpp"
#ifdef TRACY_ENABLE
#include "tracy/Tracy.hpp"
#else
#define ZoneScoped
#endif
#include "benchmark/benchmark.h"

using namespace particlesim;

template<typename Layout>
void populate_system(ParticleSystem<Layout>& ps, size_t count)
{
    for (size_t i = 0; i < count; ++i) {
        Particle p{};
        p.velocity.x = float(i % 100) * 0.01f;
        p.velocity.y = float(i % 50) * 0.01f;
        p.lifetime = 5.0f;
        ps.addParticle(p);
    }
}

template<typename Layout>
static void BM_Update(benchmark::State& state)
{
    ParticleSystem<Layout> ps(state.range(0));
    populate_system(ps, state.range(0));

    for (auto _ : state) {
        ZoneScoped;
        ps.update(0.016f, true); // simulate 1 frame (~16ms)
    }
}

BENCHMARK_TEMPLATE(BM_Update, ParticleSystemDataAoS)
    ->Name("BM_Update_AoS")
    ->Arg(1000)->Arg(10000)->Arg(50000);

BENCHMARK_TEMPLATE(BM_Update, ParticleSystemDataSoA)
    ->Name("BM_Update_SoA")
    ->Arg(1000)->Arg(10000)->Arg(50000);

BENCHMARK_MAIN();
