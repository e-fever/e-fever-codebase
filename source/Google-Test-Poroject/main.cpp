#include <gtest/gtest.h>
#include <math.h>
#include <list>

TEST(PROJECTests, test_basic) {
    ASSERT_EQ(true, true);
}

int main(int argc, char** argv) {

#ifdef QT_CORE_LIB
    static_assert(True, "Qt Core is defined");
#endif
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
