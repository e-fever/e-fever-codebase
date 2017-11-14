#include <QString>
#include <QtTest>
#include <TestRunner>
#include <QtQuickTest/quicktest.h>
#include <QtShell>
#include "snapshottests.h"
#include "snapshottesting.h"

#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
#include <execinfo.h>
#include <unistd.h>
#include <signal.h>
void handleBacktrace(int sig) {
    void *array[100];
    size_t size;

    // get void*'s for all entries on the stack
    size = backtrace(array, 100);

    // print out all the frames to stderr
    fprintf(stderr, "Error: signal %d:\\n", sig);
    backtrace_symbols_fd(array, size, STDERR_FILENO);
    exit(1);
}
#endif

namespace AutoTestRegister {
QUICK_TEST_MAIN(QuickTests)
}

int main(int argc, char *argv[])
{
#if defined(Q_OS_MAC) || defined(Q_OS_LINUX)
    signal(SIGSEGV, handleBacktrace);
#endif

    qputenv("QML_DISABLE_DISK_CACHE", "1");

    QGuiApplication app(argc, argv);

    if (qgetenv("TRAVIS") == "true") {
        SnapshotTesting::setInteractiveEnabled(false);
    }

    if (qgetenv("APPVEYOR") == "True") {
        SnapshotTesting::setInteractiveEnabled(false);
    }

    SnapshotTesting::setSnapshotsFile(QtShell::realpath_strip(SRCDIR, "snapshots.json"));

    TestRunner runner;
    runner.addImportPath("qrc:///");
    runner.add<SnapshotTests>();
    runner.add(QString(SRCDIR) + "qmltests");

    bool error = runner.exec(app.arguments());

    if (!error) {
        qDebug() << "All test cases passed!";
    }

    return error;
}
