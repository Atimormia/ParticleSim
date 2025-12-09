#pragma once
#include <span>
#include <vector>
#include "math/vector.hpp"
namespace particlesim
{

    struct ISpatialPartition
    {
        virtual ~ISpatialPartition() = default;

        virtual void build(std::span<const math::Vector2D> positions) = 0;

        virtual std::span<const uint32_t> queryNeighborhood(uint32_t particleID) const = 0;

        virtual void clear() = 0;
    };

    struct UniformGridConfig
    {
        float cellSize = 1.f;
        uint32_t gridWidth = 100;
        uint32_t gridHeight = 100;
    };

    class UniformGrid final : public ISpatialPartition
    {
        std::vector<std::vector<uint32_t>> buckets;
        UniformGridConfig config;

    public:
        UniformGrid(const UniformGridConfig &cfg);

        void resizeGrid(float newWorldWidth, float newWorldHeight);

        uint32_t toCellIndex(float x, float y) const;

        void build(std::span<const math::Vector2D> positions) override;

        std::span<const uint32_t> queryNeighborhood(uint32_t particleID) const override;

        void clear() override;
    };

}
