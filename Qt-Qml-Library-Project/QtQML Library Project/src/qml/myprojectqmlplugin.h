#pragma once

#include <QtQml/QQmlExtensionPlugin>

class %{ProjectName}QmlPlugin : public QQmlExtensionPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID QQmlExtensionInterface_iid)

public:
    void registerTypes(const char *uri);
};

