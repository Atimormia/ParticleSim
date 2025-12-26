#pragma once
#include <span>
#include <vector>
#include <cstdint>
#include <cstdio>
#include "core/vector.hpp"
#include "core/memory_arena.hpp"
namespace particlesim
{
    using namespace core;
    using namespace std;
    struct WorldBounds
    {
        float minX = 0.f;
        float minY = 0.f;
        float maxX = 100.f;
        float maxY = 100.f;

        float width() const { return maxX - minX; }
        float height() const { return maxY - minY; }
    };
    struct PartitioningConfig
    {
        float cellSize = 1.f;             // world units per cell
        WorldBounds world = {};           // world bounds
        bool excludeSelfFromQuery = true; // whether queryNeighborhood filters out queried particle
        size_t neighborReserve = 256;     // reserve size for neighbor buffer
    };

    struct PartitionData
    {
        span<const Vector2D> positions = {};
        FrameArena arena = {};
    };
    class ISpatialPartition
    {
    public: 
        virtual ~ISpatialPartition() = default;
        virtual void setData(const PartitionData &data) = 0;
        virtual void build() = 0;
        virtual span<const uint32_t> queryNeighborhood(uint32_t particleID) = 0;
        virtual void clear() = 0;
    };

    class UniformGrid : public ISpatialPartition
    {
    public:
        UniformGrid(const PartitioningConfig &cfg);

        // update grid dimensions when world bounds or cellSize change
        void resizeGrid(float cellSize, const WorldBounds &world);

        // ISpatialPartition interface
        void setData(const PartitionData &data) override { this->data = data; }
        virtual void build() override;
        virtual span<const uint32_t> queryNeighborhood(uint32_t particleID) override;
        virtual void clear() override;

        uint32_t toCellIndex(float x, float y) const;
        void worldToCell(float x, float y, int &outX, int &outY) const;

    protected:
        PartitionData data = {};
        uint32_t gridWidth = 0;
        uint32_t gridHeight = 0;
        PartitioningConfig config;

    private:
        WorldBounds bounds;

        vector<vector<uint32_t>> buckets;
        mutable vector<uint32_t> neighborBuffer;

        void ensureBucketsSize();
    };
    class UniformGridAllocated : public UniformGrid
    {
    public:
        UniformGridAllocated(const PartitioningConfig &cfg) : UniformGrid(cfg) {};

        void build() override;
        span<const uint32_t> queryNeighborhood(uint32_t particleID) override;
        void clear() override;

    private:
        struct BucketInfo
        {
            uint32_t *data;
            uint32_t count;
            uint32_t capacity;
        };
        BucketInfo *buckets = nullptr;
    };

    class NoPartition final : public ISpatialPartition
    {
    public:
        explicit NoPartition(const PartitioningConfig &cfg) : config(cfg) { neighborBuffer.reserve(cfg.neighborReserve); }

        void setData(const PartitionData &data) override { this->data = data; }

        void build() override {}

        span<const uint32_t> queryNeighborhood(uint32_t particleID) override;

        void clear() override { neighborBuffer.clear(); }

    private:
        PartitioningConfig config;
        PartitionData data = {};
        mutable vector<uint32_t> neighborBuffer;
    };

}
