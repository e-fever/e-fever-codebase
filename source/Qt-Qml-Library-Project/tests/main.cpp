#include <QString>
#include <QtTest>
#include <TestRunner>
#include <QtQuickTest/quicktest.h>
#include <XBacktrace.h>
#include <QtShell>
#include "testcases.h"

/// Include the Dummy class from src/cpp. You may remove this line from your library
#include "dummy.h"

namespace AutoTestRegister { // For Qt Creator's auto test plugin
    QUICK_TEST_MAIN(QuickTests)
}

int main(int argc, char *argv[])
{
    XBacktrace::enableBacktraceLogOnUnhandledException();

    QGuiApplication app(argc, argv);

    Q_INIT_RESOURCE(MYPROJECT);

    TestRunner runner;
    runner.add<TestCases>();
    runner.add<Dummy>();
    runner.add(QString(SRCDIR) + "qmltests");

    bool error = runner.exec(app.arguments());

    if (!error) {
        qDebug() << "All test cases passed!";
    }

    return error;
}
