#include <gtest/gtest.h>

TEST(MyTest, MyTestDownload) {
    EXPECT_EQ(4*4, 16);
    EXPECT_EQ(4*4, 15);
}

int main (int argc, char *argv[]) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
