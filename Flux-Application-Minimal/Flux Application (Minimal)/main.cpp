#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <XBacktrace.h>
#include "appview.h"

int main(int argc, char *argv[])
{
    qputenv("QML_DISABLE_DISK_CACHE", "true");

    XBacktrace::enableBacktraceLogOnUnhandledException();

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication app(argc, argv);
    Q_UNUSED(app);

    AppView view;
    view.start();

    return app.exec();
}
