#include <string>

#include "threadpool.h"
#include "gtest/gtest.h"

using namespace std;

namespace  {
    TEST(AnyTest , ConstructWithInt) {
        Any a(10);
        EXPECT_EQ(a.cast_<int>(), 10);
    }

    TEST(AnyTest , ConstructWithString) {
        Any a(string("aaaa"));
        EXPECT_EQ(a.cast_<string>(), string("aaaa"));
    }

    TEST(AnyTest, CopyWithInt) {
        Any a(10);
        Any b;
        b = std::move(a);
        EXPECT_EQ(b.cast_<int>(), 10);
    }

    TEST(AnyTest, CopyWithString) {
        Any a(string("aaaa"));
        Any b;
        b = std::move(a);
        EXPECT_EQ(b.cast_<string>(), "aaaa");
    }

    TEST(AnyTest, ConstructCopyWithInt) {
        Any a(10);
        Any b = std::move(a);
        EXPECT_EQ(b.cast_<int>(), 10);
    }

    TEST(AnyTest, ConstruectCopyWithString) {
        Any a(string("aaaa"));
        Any b = std::move(a);
        EXPECT_EQ(b.cast_<string>(), "aaaa");
    }

}

