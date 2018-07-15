#include <QtQml>
#include "%{ProjectName}qmlplugin.h"

static QObject *provider(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(scriptEngine);
    Q_UNUSED(engine);

    QObject* object = new QObject();
    object->setObjectName("Singleton");

    return object;
}

static void init() {
    bool registered = false;
    if (registered) {
        return;
    }
    registered = true;

    qmlRegisterSingletonType<QObject>("%{Package}", 1, 0, "%{Package}Singleton", provider);
}

Q_COREAPP_STARTUP_FUNCTION(init)

void %{ProjectName}QmlPlugin::registerTypes(const char *uri) {
    Q_UNUSED(uri);
    init();
}

#define DECLARE_LIBRARY(name) void __3rdparty_qt_static_library_entry_ ## name() {}

/// Prevent the init() function to be discarded during static linking
DECLARE_LIBRARY(%{Package})


