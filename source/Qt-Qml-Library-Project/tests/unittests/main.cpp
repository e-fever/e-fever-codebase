#include <QString>
#include <QtTest>
#include <TestRunner>
#include <QtQuickTest/quicktest.h>
#include <XBacktrace.h>
#include "tests.h"

namespace AutoTestRegister { // For Qt Creator's auto test plugin
    QUICK_TEST_MAIN(QuickTests)
}

int main(int argc, char *argv[])
{
    XBacktrace::enableBacktraceLogOnUnhandledException();

    QGuiApplication app(argc, argv);

    TestRunner runner;
    runner.addImportPath("qrc:///");
    runner.add<Tests>();
    runner.add(QString(SRCDIR) + "qmltests");

    bool error = runner.exec(app.arguments());

    if (!error) {
        qDebug() << "All test cases passed!";
    }

    return error;
}
