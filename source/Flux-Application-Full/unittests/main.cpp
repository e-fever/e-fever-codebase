#include <QString>
#include <QtTest>
#include <TestRunner>
#include <QtQuickTest/quicktest.h>
#include <snapshottesting.h>
#include <XBacktrace.h>
#include "testcases.h"

namespace AutoTestRegister {
QUICK_TEST_MAIN(QuickTests)
}

int main(int argc, char *argv[])
{
    XBacktrace::enableBacktraceLogOnUnhandledException();
    qputenv("QML_DISABLE_DISK_CACHE", "true");

    QGuiApplication app(argc, argv);

    SnapshotTesting::setSnapshotsFile(QString(SRCDIR) + "/snapshot.json");

    TestRunner runner;
    runner.addImportPath("qrc:///");
    runner.add<TestCases>();
    runner.add(QString(SRCDIR) + "qmltests");

    bool error = runner.exec(app.arguments());

    if (!error) {
        qDebug() << "All test cases passed!";
    }

    return error;
}
