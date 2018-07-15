#include <QtQml>

static QObject *provider(QQmlEngine *engine, QJSEngine *scriptEngine) {
    Q_UNUSED(scriptEngine);

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

    qmlRegisterSingletonType<QObject>("MYPACKAGE", 1, 0, "MYPACKAGESingleton", provider);
}

Q_COREAPP_STARTUP_FUNCTION(init)

#define DECLARE_LIBRARY(name) void __3rdparty_qt_static_library_entry_ ## name() {}

/// Prevent the init() function to be discarded during static linking
DECLARE_LIBRARY(MYPACKAGE)


