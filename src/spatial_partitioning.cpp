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

void UniformGrid::build()
{
    if (data.positions.empty())
        return;

    for (uint32_t i = 0; i < data.positions.size(); ++i)
    {
        auto &p = data.positions[i];
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

span<const uint32_t> UniformGrid::queryNeighborhood(uint32_t particleID)
{
    assert(data.positions.data() != nullptr && "setData() must be called before queryNeighborhood()");
    assert(particleID < data.positions.size());

    const auto &pos = data.positions[particleID];
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

void particlesim::UniformGridAllocated::build()
{
    const auto &positions = data.positions;
    const size_t particleCount = positions.size();
    if (particleCount == 0)
        return;

    const uint32_t cellCount = gridWidth * gridHeight;
    resizeGrid(config.cellSize, config.world);

    if (buckets.size() != cellCount)
        buckets.resize(cellCount);
    for (auto &b : buckets)
        b.clear();

    FrameArena* arena = data.arena;
    if (!arena)
        return;
    arena->reset();

    uint32_t* cellCounts = arena->allocateArray<uint32_t>(cellCount);
    uint32_t* cellOffsets = arena->allocateArray<uint32_t>(cellCount + 1);
    uint32_t* particleIDs = arena->allocateArray<uint32_t>(particleCount);
    memset(cellCounts, 0, cellCount * sizeof(uint32_t));

    for (size_t i = 0; i < particleCount; ++i)
    {
        const Vector2D &p = positions[i];
        uint32_t cell = toCellIndex(p.x, p.y);
        cellCounts[cell]++;
    }

    cellOffsets[0] = 0;
    for (uint32_t c = 0; c < cellCount; ++c)
    {
        cellOffsets[c + 1] = cellOffsets[c] + cellCounts[c];
        cellCounts[c] = 0; // reuse cellCounts as cursors
    }

    for (size_t i = 0; i < particleCount; ++i)
    {
        const Vector2D &p = positions[i];
        uint32_t cell = toCellIndex(p.x, p.y);

        uint32_t index = cellOffsets[cell] + cellCounts[cell]++;
        particleIDs[index] = static_cast<uint32_t>(i);
    }

    for (uint32_t c = 0; c < cellCount; ++c)
    {
        buckets[c].data = particleIDs + cellOffsets[c];
        buckets[c].count = cellCounts[c];
        buckets[c].capacity = cellCounts[c];
    }
}

void particlesim::UniformGridAllocated::clear()
{
    data.arena->reset();
}

void particlesim::UniformGridParallel::build()
{
    const auto &positions = data.positions;
    const size_t particleCount = positions.size();

    if (particleCount == 0)
        return;
    if (!data.scheduler)
        return;

    const uint32_t cellCount = gridWidth * gridHeight;
    resizeGrid(config.cellSize, config.world);
    if (buckets.size() != cellCount)
        buckets.resize(cellCount);

    for (auto &b : buckets)
        b.clear();

    auto &scheduler = *data.scheduler;
    FrameArena *arena = data.arena;

    arena->reset();

    uint32_t *cellCounts = arena->allocateArray<uint32_t>(cellCount);
    uint32_t *cellOffsets = arena->allocateArray<uint32_t>(cellCount + 1);
    uint32_t *cellCursors = arena->allocateArray<uint32_t>(cellCount);
    uint32_t *particleIDs = arena->allocateArray<uint32_t>(particleCount);

    memset(cellCounts, 0, cellCount * sizeof(uint32_t));

    scheduler.parallelFor(0, particleCount, 256, [&](size_t i)
    {
        const Vector2D &p = positions[i];
        uint32_t cell = toCellIndex(p.x, p.y);
        std::atomic_ref<uint32_t>(cellCounts[cell]).fetch_add(1, std::memory_order_relaxed);
    });

    scheduler.wait();

    cellOffsets[0] = 0;
    for (uint32_t c = 0; c < cellCount; ++c)
    {
        cellOffsets[c + 1] = cellOffsets[c] + cellCounts[c];
        cellCursors[c] = cellOffsets[c]; // reset cursors
    }

    scheduler.parallelFor(0, particleCount, 256,
    [&](size_t i)
    {
        const Vector2D &p = positions[i];
        uint32_t cell = toCellIndex(p.x, p.y);

        uint32_t index = std::atomic_ref<uint32_t>(cellCounts[cell]).fetch_add(1, std::memory_order_relaxed);

        particleIDs[index] = static_cast<uint32_t>(i);
    });

    scheduler.wait();

    for (uint32_t c = 0; c < cellCount; ++c)
    {
        buckets[c].data = particleIDs + cellOffsets[c];
        buckets[c].count = cellCounts[c];
        buckets[c].capacity = cellCounts[c];
    }
}

span<const uint32_t> particlesim::NoPartition::queryNeighborhood(uint32_t particleID)
{
    neighborBuffer.clear();

    const uint32_t count = static_cast<uint32_t>(data.positions.size());
    for (uint32_t i = 0; i < count; ++i)
    {
        if (config.excludeSelfFromQuery && i == particleID)
            continue;

        neighborBuffer.push_back(i);
    }

    return neighborBuffer;
}