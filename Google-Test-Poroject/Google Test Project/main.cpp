#include <gtest/gtest.h>
#include <QCoreApplication>

TEST(%{ProjectName}ests, test_basic) {
    ASSERT_EQ(true, true);
}

int main(int argc, char** argv) {

    QCoreApplication app(argc, argv);
    Q_UNUSED(app);

    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
