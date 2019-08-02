#include <QString>
#include <QtTest>
#include <QtQuickTest/quicktest.h>
#include <XBacktrace.h>

namespace AutoTestRegister {
QUICK_TEST_MAIN(QuickTests)
}

int main(int argc, char *argv[])
{
    XBacktrace::enableBacktraceLogOnUnhandledException();

    QGuiApplication app(argc, argv);
    return quick_test_main(argc, argv, "qmltests", TEST_SOURCE_DIR);
}
