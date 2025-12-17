#include "particlesim/spatial_partitioning.hpp"
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cmath>

using namespace particlesim;

UniformGrid::UniformGrid(const PartitioningConfig &cfg) : config(cfg), bounds(cfg.world)
{
    resizeGrid(cfg.cellSize, cfg.world);
    neighborBuffer.reserve(cfg.neighborReserve);
}

void UniformGrid::resizeGrid(float cellSize, const WorldBounds &world)
{
    assert(cellSize > 0.f);
    config.cellSize = cellSize;
    bounds = world;

    gridWidth = max<int>(1, ceil(bounds.width() / config.cellSize));
    gridHeight = max<int>(1, ceil(bounds.height() / config.cellSize));
    ensureBucketsSize();
    neighborBuffer.reserve(config.neighborReserve);
}

void UniformGrid::ensureBucketsSize()
{
    buckets.clear();
    buckets.resize(gridWidth * gridHeight);
}

void UniformGrid::setPositions(span<const Vector2D> pos)
{
    positions = pos;
}

void UniformGrid::build()
{
    for (auto &b : buckets)
        b.clear();
    if (positions.empty())
        return;

    for (uint32_t i = 0; i < positions.size(); ++i)
    {
        auto &p = positions[i];
        uint32_t idx = toCellIndex(p.x, p.y);
        buckets[idx].push_back(i);
    }
}

uint32_t UniformGrid::toCellIndex(float x, float y) const
{
    int cx, cy;
    worldToCell(x, y, cx, cy);
    // clamp safety
    cx = clamp(cx, 0, static_cast<int>(gridWidth) - 1);
    cy = clamp(cy, 0, static_cast<int>(gridHeight) - 1);
    return static_cast<uint32_t>(cy * gridWidth + cx);
}

void UniformGrid::worldToCell(float x, float y, int &outX, int &outY) const
{
    // convert world coordinate to cell coordinate - relative to bounds.min
    float nx = (x - bounds.minX) / config.cellSize;
    float ny = (y - bounds.minY) / config.cellSize;
    outX = static_cast<int>(floor(nx));
    outY = static_cast<int>(floor(ny));
}

span<const uint32_t> UniformGrid::queryNeighborhood(uint32_t particleID) const
{
    assert(positions.data() != nullptr && "setPositions() must be called before queryNeighborhood()");
    assert(particleID < positions.size());

    const auto &pos = positions[particleID];
    int cx, cy;
    worldToCell(pos.x, pos.y, cx, cy);

    neighborBuffer.clear();

    for (int dy = -1; dy <= 1; ++dy)
    {
        int ny = cy + dy;
        if (ny < 0 || ny >= static_cast<int>(gridHeight))
            continue;
        for (int dx = -1; dx <= 1; ++dx)
        {
            int nx = cx + dx;
            if (nx < 0 || nx >= static_cast<int>(gridWidth))
                continue;
            uint32_t cellIdx = ny * gridWidth + nx;
            const auto &bucket = buckets[cellIdx];

            neighborBuffer.insert(neighborBuffer.end(), bucket.begin(), bucket.end());
        }
    }

    if (config.excludeSelfFromQuery)
    {
        // buckets are small so linear search is fine
        for (size_t i = 0; i < neighborBuffer.size(); ++i)
        {
            // remove the queried particle from neighborBuffer in-place if present
            if (neighborBuffer[i] == particleID)
            {
                neighborBuffer[i] = neighborBuffer.back();
                neighborBuffer.pop_back();
                break;
            }
        }
    }

    return {neighborBuffer.data(), neighborBuffer.size()};
}

void UniformGrid::clear()
{
    for (auto &b : buckets)
        b.clear();
    neighborBuffer.clear();
}

span<const uint32_t> particlesim::NoPartition::queryNeighborhood(uint32_t particleID) const
{
    neighborBuffer.clear();

    const uint32_t count = static_cast<uint32_t>(positions.size());
    for (uint32_t i = 0; i < count; ++i)
    {
        if (config.excludeSelfFromQuery && i == particleID)
            continue;

        neighborBuffer.push_back(i);
    }

    return neighborBuffer;
}