#include <iostream>
#include "particlesim/particle_system.hpp"
#include <chrono>
#include <functional>
#include <random>
#include "benchmark_helper.hpp"

using namespace particlesim;

Particle generate_particle(
    float area_half_size = 50.0f,
    float max_speed = 5.0f,
    float max_acc = 1.0f,
    float min_life = 1.0f,
    float max_life = 10.0f)
{
    static thread_local std::mt19937 rng(std::random_device{}());

    std::uniform_real_distribution<float> pos_dist(-area_half_size, area_half_size);
    std::uniform_real_distribution<float> vel_dist(-max_speed, max_speed);
    std::uniform_real_distribution<float> acc_dist(-max_acc, max_acc);
    std::uniform_real_distribution<float> life_dist(min_life, max_life);

    Particle p;

    p.position = {pos_dist(rng), pos_dist(rng)};
    p.velocity = {vel_dist(rng), vel_dist(rng)};
    p.acceleration = {acc_dist(rng), acc_dist(rng)};
    p.lifetime = life_dist(rng);
    p.alive = true;

    return p;
}

int main()
{
    ParticleSystem ps(10000000);

    Particle p1;
    p1.velocity.x = 1.0f;
    p1.lifetime = 10.0f;
    Particle p2;
    p2.velocity.x = 0.5f;
    p2.lifetime = 2.0f;
    Particle p3;
    p3.velocity.x = -0.25f;
    p3.lifetime = 15.0f;

    ps.add_particle(p1);
    ps.add_particle(p2);
    ps.add_particle(p3);

    for (int i = 0; i < 10; i++)
    {
        ps.update(1.0f);
    }

    std::cout << ps.tostring() << std::endl;

    for (int i = 0; i < 1'000'000; i++)
    {
        ps.add_particle(generate_particle());
    }
    std::cout << ps.particles().size() << " particles" << std::endl;
    
    auto result = BenchmarkHelper::run([&]() {
        ps.update(0.016f); // simulate 16ms frame
    });
    std::cout << result.tostring() << std::endl;
    return 0;
}
