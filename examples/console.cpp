#include <iostream>
#include "particlesim/particle_system.hpp"

using namespace particlesim;

int main() {
    ParticleSystem ps(1000);

    Particle p1; p1.velocity.x = 1.0f;
    Particle p2; p2.velocity.x = 0.5f;
    Particle p3; p3.velocity.x = -0.25f;

    ps.add_particle(p1);
    ps.add_particle(p2);
    ps.add_particle(p3);

    for (int i = 0; i < 10; i++) {
        ps.update(1.0f);
    }

    const auto& arr = ps.particles();
    for (size_t i = 0; i < arr.size(); i++) {
        std::cout << "Particle " << i << " final x: " << arr[i].position.x << "\n";
    }

    return 0;
}
