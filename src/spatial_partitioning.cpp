#include "spatial_partitioning.hpp"

particlesim::UniformGrid::UniformGrid(const UniformGridConfig &cfg)
{
    
}

void particlesim::UniformGrid::resizeGrid(float newWorldWidth, float newWorldHeight)
{
}

uint32_t particlesim::UniformGrid::toCellIndex(float x, float y) const
{
    return 0;
}

void particlesim::UniformGrid::build(std::span<const math::Vector2D> positions)
{
}

std::span<const uint32_t> particlesim::UniformGrid::queryNeighborhood(uint32_t particleID) const
{
    return std::span<const uint32_t>();
}

void particlesim::UniformGrid::clear()
{
}
