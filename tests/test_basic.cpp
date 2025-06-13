#include "pch.h"

// Simple test to verify Google Test is working
TEST(BasicTest, AlwaysPass) {
  EXPECT_EQ(1, 1);
  EXPECT_TRUE(true);
}

TEST(BasicTest, SimpleArithmetic) {
  EXPECT_EQ(2 + 2, 4);
  EXPECT_NE(3 + 3, 7);
}