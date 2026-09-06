#include <gtest/gtest.h>
#include <vecdb/distance.hpp>

TEST(DistanceTest, DifferentDimensionsError) {
    std::vector<float> first{1, 2};
    std::vector<float> second{1, 2, 3};
    EXPECT_THROW(vecdb::squared_l2_distance(first, second), std::invalid_argument);
}

TEST(DistanceTest, Distance_with_self_is_zero) { // if anyones reading this feel free to convert
                                                 // this to PascalCase- due to inexplicable
                                                 // circumstances i cannot type a capital w rn
    std::vector<float> first{67.8, 69.5, 420.03};
    EXPECT_FLOAT_EQ(vecdb::squared_l2_distance(first, first), 0);
}

TEST(DistanceTest, CorrectL2Distance) {
    std::vector<float> first{2.0, 4.0, 6.0, 8.0, 11.29874591};
    std::vector<float> second{1.0, 2.0, 3.0, 4.0, 5.0};
    EXPECT_FLOAT_EQ(vecdb::squared_l2_distance(first, second), 69.67420);
}