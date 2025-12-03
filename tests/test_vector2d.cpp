#include <gtest/gtest.h>
#include "math/vector.hpp"

using namespace math;

TEST(Vector2D, DefaultConstructor) {
    Vector2D v;
    EXPECT_FLOAT_EQ(v.x, 0.0f);
    EXPECT_FLOAT_EQ(v.y, 0.0f);
}

TEST(Vector2D, ParameterConstructor) {
    Vector2D v(3.0f, -2.0f);
    EXPECT_FLOAT_EQ(v.x, 3.0f);
    EXPECT_FLOAT_EQ(v.y, -2.0f);
}

TEST(Vector2D, Addition) {
    Vector2D a(1.0f, 2.0f);
    Vector2D b(3.0f, -1.0f);
    Vector2D c = a + b;

    EXPECT_FLOAT_EQ(c.x, 4.0f);
    EXPECT_FLOAT_EQ(c.y, 1.0f);
}

TEST(Vector2D, InPlaceAddition) {
    Vector2D a(1.0f, 1.0f);
    a += Vector2D(2.0f, 3.0f);

    EXPECT_FLOAT_EQ(a.x, 3.0f);
    EXPECT_FLOAT_EQ(a.y, 4.0f);
}

TEST(Vector2D, Subtraction) {
    Vector2D a(5.0f, 4.0f);
    Vector2D b(2.0f, 1.0f);
    Vector2D c = a - b;

    EXPECT_FLOAT_EQ(c.x, 3.0f);
    EXPECT_FLOAT_EQ(c.y, 3.0f);
}

TEST(Vector2D, InPlaceSubtraction) {
    Vector2D a(4.0f, 3.0f);
    a -= Vector2D(1.0f, 1.0f);

    EXPECT_FLOAT_EQ(a.x, 3.0f);
    EXPECT_FLOAT_EQ(a.y, 2.0f);
}

TEST(Vector2D, ScalarMultiplication) {
    Vector2D a(2.0f, -3.0f);
    Vector2D b = a * 2.0f;

    EXPECT_FLOAT_EQ(b.x, 4.0f);
    EXPECT_FLOAT_EQ(b.y, -6.0f);
}

