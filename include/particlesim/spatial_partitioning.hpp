#pragma once
#include <span>
#include <vector>
#include "math/vector.hpp"
namespace particlesim
{
    struct WorldBounds
    {
        float minX = 0.f;
        float minY = 0.f;
        float maxX = 100.f;
        float maxY = 100.f;

        float width() const { return maxX - minX; }
        float height() const { return maxY - minY; }
    };
    struct UniformGridConfig
    {
        float cellSize = 1.f;// world units per cell
        WorldBounds world = {};// world bounds
        bool excludeSelfFromQuery = true;// whether queryNeighborhood filters out queried particle
        size_t neighborReserve = 256;// reserve size for neighbor buffer
    };

    class ISpatialPartition
    {
    public:
        virtual ~ISpatialPartition() = default;
        virtual void setPositions(std::span<const math::Vector2D> positions) = 0;
        virtual void build() = 0;
        virtual std::span<const uint32_t> queryNeighborhood(uint32_t particleID) const = 0;
        virtual void clear() = 0;
    };

    class UniformGrid final : public ISpatialPartition
    {
    public:
        UniformGrid(const UniformGridConfig &cfg);

        // update grid dimensions when world bounds or cellSize change
        void resizeGrid(float cellSize, const WorldBounds &world);

        // ISpatialPartition interface
        void setPositions(std::span<const math::Vector2D> positions) override;
        void build() override;
        std::span<const uint32_t> queryNeighborhood(uint32_t particleID) const override;
        void clear() override;

        uint32_t toCellIndex(float x, float y) const;
        void worldToCell(float x, float y, int &outX, int &outY) const;

    private:
        UniformGridConfig config;
        WorldBounds bounds;
        std::span<const math::Vector2D> positions = {};
        uint32_t gridWidth = 0;
        uint32_t gridHeight = 0;

        std::vector<std::vector<uint32_t>> buckets;
        mutable std::vector<uint32_t> neighborBuffer;

        void ensureBucketsSize();
    };

}
