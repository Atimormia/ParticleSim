#include <gtest/gtest.h>
#include "particlesim/spatial_partitioning.hpp"

using namespace particlesim;

TEST(UniformGrid, InitializesGridCorrectly)
{
    UniformGridConfig cfg;
    cfg.cellSize = 10.f;
    cfg.world = {0, 0, 100, 50};

    UniformGrid grid(cfg);

    // gridWidth = ceil(100/10) = 10
    // gridHeight = ceil(50/10) = 5
    EXPECT_EQ(grid.toCellIndex(0.f, 0.f), 0);
    EXPECT_EQ(grid.toCellIndex(99.f, 49.f), (5 - 1) * 10 + (10 - 1));
}

TEST(UniformGrid, SetPositionsStoresSpan)
{
    UniformGridConfig cfg;
    UniformGrid grid(cfg);

    std::vector<math::Vector2D> pos = {{1, 2}, {3, 4}};
    grid.setPositions(pos);

    grid.build(); // should not crash
}

TEST(UniformGrid, BuildBinsParticlesIntoCorrectCells)
{
    UniformGridConfig cfg;
    cfg.cellSize = 10.f;
    cfg.world = {0, 0, 100, 100};
    cfg.excludeSelfFromQuery = false;

    UniformGrid grid(cfg);

    std::vector<math::Vector2D> pos = {
        {5, 5},  // cell (0,0)
        {15, 5}, // cell (1,0)
        {5, 15}, // cell (0,1)
        {95, 95} // bottom-right cell
    };

    grid.setPositions(pos);
    grid.build();

    // Check that each particle is in the correct bucket
    auto idx0 = grid.toCellIndex(pos[0].x, pos[0].y);
    auto idx1 = grid.toCellIndex(pos[1].x, pos[1].y);
    auto idx2 = grid.toCellIndex(pos[2].x, pos[2].y);
    auto idx3 = grid.toCellIndex(pos[3].x, pos[3].y);

    // Query each particle and ensure it is present in its own bucket
    auto neigh0 = grid.queryNeighborhood(0);
    EXPECT_NE(std::find(neigh0.begin(), neigh0.end(), 0), neigh0.end());

    auto neigh1 = grid.queryNeighborhood(1);
    EXPECT_NE(std::find(neigh1.begin(), neigh1.end(), 1), neigh1.end());

    auto neigh2 = grid.queryNeighborhood(2);
    EXPECT_NE(std::find(neigh2.begin(), neigh2.end(), 2), neigh2.end());

    auto neigh3 = grid.queryNeighborhood(3);
    EXPECT_NE(std::find(neigh3.begin(), neigh3.end(), 3), neigh3.end());
}

TEST(UniformGrid, QueryNeighborhoodReturnsNeighborsIn8ConnectedRegion)
{
    UniformGridConfig cfg;
    cfg.cellSize = 10.f;
    cfg.world = {0, 0, 30, 30};
    cfg.excludeSelfFromQuery = false;

    UniformGrid grid(cfg);

    // Particles arranged so particle 4 sits in center cell (1,1)
    std::vector<math::Vector2D> pos = {
        {5, 5}, {15, 5}, {25, 5}, {5, 15}, {15, 15}, {25, 15}, {5, 25}, {15, 25}, {25, 25}};

    grid.setPositions(pos);
    grid.build();

    auto neigh = grid.queryNeighborhood(4);

    // Should get all 9 particles
    EXPECT_EQ(neigh.size(), 9);
    for (uint32_t i = 0; i < 9; i++)
    {
        EXPECT_NE(std::find(neigh.begin(), neigh.end(), i), neigh.end());
    }
}

TEST(UniformGrid, ExcludeSelfRemovesSelfFromResult)
{
    UniformGridConfig cfg;
    cfg.excludeSelfFromQuery = true;

    UniformGrid grid(cfg);
    std::vector<math::Vector2D> pos = {{1, 1}, {1.1f, 1.1f}};
    grid.setPositions(pos);
    grid.build();

    auto neigh = grid.queryNeighborhood(0);
    EXPECT_EQ(neigh.size(), 1);
    EXPECT_EQ(neigh[0], 1);

    auto neigh2 = grid.queryNeighborhood(1);
    EXPECT_EQ(neigh2.size(), 1);
    EXPECT_EQ(neigh2[0], 0);
}

TEST(UniformGrid, QueryNeighborhoodEdgesDoNotGoOutOfBounds)
{
    UniformGridConfig cfg;
    cfg.cellSize = 10.f;
    cfg.world = {0, 0, 30, 30}; 
    cfg.excludeSelfFromQuery = false;

    UniformGrid grid(cfg);

    std::vector<math::Vector2D> pos = {
        {1, 1},  // top-left corner cell
        {11, 1}, // top-center
        {1, 11}  // center-left
    };

    grid.setPositions(pos);
    grid.build();

    auto neighSpan = grid.queryNeighborhood(0);
    std::vector<uint32_t> neigh(neighSpan.begin(), neighSpan.end());

    std::vector<uint32_t> expected = {0, 1, 2};
    EXPECT_EQ(neigh.size(), expected.size());
    for (size_t i = 0; i < expected.size(); ++i)
    {
        EXPECT_EQ(neigh[i], expected[i]);
    }
}

TEST(UniformGrid, ResizeGridRebuildsCorrectDimensions)
{
    UniformGridConfig cfg;
    cfg.cellSize = 5.f;
    cfg.world = {0, 0, 20, 20};

    UniformGrid grid(cfg);

    // initial grid is 4x4
    EXPECT_EQ(grid.toCellIndex(19.f, 19.f), 15);

    // resize to 10-unit cells
    grid.resizeGrid(10.f, {0, 0, 20, 20});
    // new grid 2x2
    EXPECT_EQ(grid.toCellIndex(19.f, 19.f), 3);
}

TEST(UniformGrid, ClearEmptiesBucketsAndBuffer)
{
    UniformGridConfig cfg;
    UniformGrid grid(cfg);

    std::vector<math::Vector2D> pos = {{1, 1}, {2, 2}};
    grid.setPositions(pos);
    grid.build();

    auto neigh = grid.queryNeighborhood(0);
    EXPECT_GT(neigh.size(), 0);

    grid.clear();

    auto neigh2 = grid.queryNeighborhood(0);
    // because build has not been called again, now each bucket is empty
    EXPECT_EQ(neigh2.size(), 0);
}

TEST(UniformGrid, ParticlesOutsideBoundsAreClamped)
{
    UniformGridConfig cfg;
    cfg.cellSize = 10.f;
    cfg.world = {0, 0, 100, 100};

    UniformGrid grid(cfg);

    std::vector<math::Vector2D> pos = {
        {-10, -10}, // should clamp to cell (0,0)
        {150, 150}  // should clamp to last cell
    };

    grid.setPositions(pos);
    grid.build();

    auto idx0 = grid.toCellIndex(pos[0].x, pos[0].y);
    auto idx1 = grid.toCellIndex(pos[1].x, pos[1].y);

    EXPECT_EQ(idx0, 0);
    EXPECT_EQ(idx1, (10 - 1) + (10 - 1) * 10);
}
