#include "particlesim/spatial_partitioning.hpp"
#include <algorithm>
#include <assert.h>
#include <cstdint>
#include <cmath>
#include <thread>

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
    if (data->positions.empty())
        return;

    for (uint32_t i = 0; i < data->positions.size(); ++i)
    {
        auto &p = data->positions[i];
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
    assert(data->positions.data() != nullptr && "setData() must be called before queryNeighborhood()");
    assert(particleID < data->positions.size());

    const auto &pos = data->positions[particleID];
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
    size_t particleCount = data->positions.size();
    const size_t bucketCount = static_cast<size_t>(gridWidth) * gridHeight;

    assert(bucketCount <= particleCount * 8 && "UniformGrid resolution too fine for particle count");
    buckets = data->arena.allocateArray<BucketInfo>(bucketCount);

    // initialize counts
    for (size_t i = 0; i < bucketCount; ++i)
    {
        buckets[i].count = 0;
        buckets[i].capacity = 0;
        buckets[i].data = nullptr;
    }

    if (particleCount == 0)
        return;

    // count how many particles go into each bucket
    uint32_t* counts = data->arena.allocateArray<uint32_t>(bucketCount);
    std::fill_n(counts, bucketCount, 0u);

    for (uint32_t i = 0; i < particleCount; ++i)
    {
        const auto &p = data->positions[i];
        uint32_t idx = toCellIndex(p.x, p.y);
        ++counts[idx];
    }

    // allocate exact-sized arrays for each bucket
    for (size_t i = 0; i < bucketCount; ++i)
    {
        if (counts[i] > 0)
        {
            buckets[i].capacity = counts[i];
            buckets[i].data = data->arena.allocateArray<uint32_t>(buckets[i].capacity);
            buckets[i].count = 0;
        }
    }

    // populate buckets
    for (uint32_t i = 0; i < particleCount; ++i)
    {
        const auto &p = data->positions[i];
        uint32_t idx = toCellIndex(p.x, p.y);

        BucketInfo &bucket = buckets[idx];
        bucket.data[bucket.count++] = i;
    }
}

span<const uint32_t> particlesim::UniformGridAllocated::queryNeighborhood(uint32_t particleID)
{
    size_t size = data->positions.size();
    assert(data->positions.data() != nullptr && "setData() must be called before queryNeighborhood()");
    assert(particleID < size);

    const auto &pos = data->positions[particleID];
    int cx, cy;
    worldToCell(pos.x, pos.y, cx, cy);

    size_t maxNeighbors = 0;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx)
            maxNeighbors += size;

    uint32_t *result = data->arena.allocateArray<uint32_t>(maxNeighbors);
    uint32_t count = 0;

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

            const BucketInfo &bucket = buckets[ny * gridWidth + nx];
            memcpy(result + count, bucket.data,
                   bucket.count * sizeof(uint32_t));
            count += bucket.count;
        }
    }

    if (config.excludeSelfFromQuery)
    {
        for (uint32_t i = 0; i < count; ++i)
        {
            if (result[i] == particleID)
            {
                result[i] = result[--count];
                break;
            }
        }
    }

    return {result, count};
}

void particlesim::UniformGridAllocated::clear()
{
    data->arena.reset();
}

span<const uint32_t> particlesim::NoPartition::queryNeighborhood(uint32_t particleID)
{
    neighborBuffer.clear();

    const uint32_t count = static_cast<uint32_t>(data->positions.size());
    for (uint32_t i = 0; i < count; ++i)
    {
        if (config.excludeSelfFromQuery && i == particleID)
            continue;

        neighborBuffer.push_back(i);
    }

    return neighborBuffer;
}

void particlesim::UniformGridParallel::build()
{
    const auto& positions = data->positions;
    const size_t particleCount = positions.size();

    if (particleCount == 0)
        return;

    resizeGrid(config.cellSize, config.world);
    ensureBucketsSize();

    for (auto& b : bucketsParallel)
        b.clear();

    if (!data->scheduler)
        return;

    auto& scheduler = *data->scheduler;
    FrameArena& arena = data->arena;

    const uint32_t cellCount = gridWidth * gridHeight;

    // Conservative thread count guess
    const uint32_t threadCount = std::max(1u, static_cast<uint32_t>(std::thread::hardware_concurrency()));

    BucketInfo* threadBuckets = arena.allocateArray<BucketInfo>(threadCount * cellCount);
    for (uint32_t t = 0; t < threadCount; ++t)
    {
        for (uint32_t c = 0; c < cellCount; ++c)
        {
            BucketInfo& b = threadBuckets[t * cellCount + c];
            b.count = 0;
            b.capacity = 8; // small initial capacity, grows implicitly by over-allocation
            b.data = arena.allocateArray<uint32_t>(b.capacity);
        }
    }

    // Thread-local index assignment
    static thread_local uint32_t tlsThreadIndex = UINT32_MAX;
    static std::atomic<uint32_t> nextThreadIndex{0};

    auto getThreadIndex = [&]()
    {
        if (tlsThreadIndex == UINT32_MAX)
            tlsThreadIndex = nextThreadIndex.fetch_add(1);
        return tlsThreadIndex;
    };

    // --- PARALLEL FILL ---
    scheduler.parallelFor(0, particleCount, 256,
        [&](size_t i)
        {
            uint32_t tid = getThreadIndex();
            if (tid >= threadCount)
                return; // safety

            const Vector2D& p = positions[i];
            uint32_t cell = toCellIndex(p.x, p.y);

            BucketInfo& bucket = threadBuckets[tid * cellCount + cell];

            // Grow if needed
            if (bucket.count == bucket.capacity)
            {
                uint32_t newCap = bucket.capacity * 2;
                uint32_t* newData = arena.allocateArray<uint32_t>(newCap);
                memcpy(newData, bucket.data, bucket.count * sizeof(uint32_t));
                bucket.data = newData;
                bucket.capacity = newCap;
            }

            bucket.data[bucket.count++] = static_cast<uint32_t>(i);
        });

    scheduler.wait();

    // --- MERGE ---
    for (uint32_t c = 0; c < cellCount; ++c)
    {
        BucketInfo& dst = bucketsParallel[c];
        for (uint32_t t = 0; t < threadCount; ++t)
        {
            BucketInfo& src = threadBuckets[t * cellCount + c];
            if (src.count == 0)
                continue;

            size_t newCount = dst.count + src.count;
            if (newCount > dst.capacity)
            {
                uint32_t newCap = static_cast<uint32_t>(newCount * 2);
                uint32_t* newData = arena.allocateArray<uint32_t>(newCap);
                memcpy(newData, dst.data, dst.count * sizeof(uint32_t));
                dst.data = newData;
                dst.capacity = newCap;
            }

            memcpy(dst.data + dst.count, src.data, src.count * sizeof(uint32_t));
            dst.count = newCount;
        }
    }

}