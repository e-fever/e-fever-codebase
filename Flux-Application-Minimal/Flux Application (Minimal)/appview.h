#pragma once

#include <QObject>
#include <QQmlApplicationEngine>
#include <QJSValue>

/* AppView is the bridge between Application(C++) and View(QML).
 */

class AppView : public QObject
{
    Q_OBJECT
public:
    explicit AppView(QObject *parent = 0);

    void start();

signals:

public slots:

private slots:
    void onDispatched(QString type, QJSValue message);

private:

    QQmlApplicationEngine m_engine;

};
