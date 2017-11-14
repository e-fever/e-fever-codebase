#include <QTest>
#include <QtShell>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QJsonDocument>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <private/qqmldata_p.h>
#include <private/qqmlcontext_p.h>
#include "automator.h"
#include "snapshottests.h"
#include "snapshottesting.h"
#include "private/snapshottesting_p.h"

using namespace SnapshotTesting;
using namespace SnapshotTesting::Private;

SnapshotTests::SnapshotTests(QObject *parent) : QObject(parent)
{
    auto ref = [=]() {
        QTest::qExec(this, 0, 0); // Autotest detect available test cases of a QObject by looking for "QTest::qExec" in source code
    };
    Q_UNUSED(ref);
}

void SnapshotTests::init()
{
    {
        // Make sure the QtQuick package is loaded

        QQmlEngine engine;

        QString qml  = "import QtQuick 2.0\\n Item { }";

        QObject* holder = 0;

        QQmlComponent comp (&engine);
        comp.setData(qml.toUtf8(),QUrl());
        holder = comp.create();
        holder->deleteLater();
    }
}

void SnapshotTests::test_obtainQmlPackage()
{
    QQuickItem* item = new QQuickItem();
    QString package = obtainQmlPackage(item);

    QCOMPARE(package, QString("QtQuick"));
    delete item;
}

void SnapshotTests::test_obtainDynamicDefaultValues()
{
    QQuickItem* item = new QQuickItem();

    item->setX(100);
    QVariantMap defaultValues = obtainDynamicDefaultValues(item);

    QVERIFY(defaultValues.contains("x"));
    QCOMPARE(defaultValues["x"].toInt(), 0);

    delete item;

}

void SnapshotTests::test_classNameToComponentName()
{
    QCOMPARE(classNameToComponentName("AnyOtherClass"), QString("AnyOtherClass"));
    QCOMPARE(classNameToComponentName("AnyOtherClass_QML_123"), QString("AnyOtherClass"));
    QCOMPARE(classNameToComponentName("QQuickItem"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickItem_QML_123"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickItem_QML_4523"), QString("Item"));
}

void SnapshotTests::test_context()
{
    QQmlApplicationEngine engine;

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample1.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QQmlContext* context = qmlContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample1"));

        context = SnapshotTesting::Private::obtainCreationContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample1"));

        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Item"));

        QVERIFY(obtainCurrentScopeContext(object) == qmlContext(object));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample5.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QQmlContext* context = qmlContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample5"));

        context = SnapshotTesting::Private::obtainCreationContext(object);
        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample5Form"));

        QVERIFY(obtainCurrentScopeContext(object) == qmlContext(object));
        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Sample5Form"));

    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample2.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QQuickItem* child = object->findChild<QQuickItem*>("item_sample1");

        QVERIFY(obtainCurrentScopeContext(child) != qmlContext(child));
        QCOMPARE(obtainRootComponentName(object), QString("Item"));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample6.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseContext(object), QString("Sample5Form"));

        QCOMPARE(obtainComponentNameByBaseUrl(obtainCurrentScopeContext(object)->baseUrl()), QString("Sample6"));

        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Sample5"));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample7.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(obtainComponentNameByClass(object), QString("Sample7"));

        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object), QString("Item"));
        QCOMPARE(SnapshotTesting::Private::obtainRootComponentName(object, true), QString("Item"));
    }
}

void SnapshotTests::test_loading_config()
{
    {
        QString text = QtShell::cat(":/qt-project.org/imports/SnapshotTesting/config/snapshot-config.json");

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(),&error);
        Q_UNUSED(doc);

        QVERIFY(error.error == QJsonParseError::NoError);
    }
}

void SnapshotTests::test_SnapshotTesting_diff()
{
    QString text1 = "A\\nB\\nC";
    QString text2 = "A\\nD\\nC";

    QString result = SnapshotTesting::diff(text1, text2);

    qDebug().noquote() << result;
}

void SnapshotTests::test_SnapshotTesting_saveSnapshots()
{
    SnapshotTesting::saveSnapshots();
}

void SnapshotTests::test_SnapshotTesting_addClassIgnoredProperty()
{
    QString input = QtShell::realpath_strip(SRCDIR, "sample/Sample1.qml");

    QQmlApplicationEngine engine;
    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    QString name, text;

    name = QString("%1_default").arg(QTest::currentTestFunction());

    text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));

    SnapshotTesting::addClassIgnoredProperty("QQuickRectangle", "width");
    name = QString("%1_set").arg(QTest::currentTestFunction());

    text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));

    SnapshotTesting::removeClassIgnoredProperty("QQuickRectangle", "width");
    name = QString("%1_default").arg(QTest::currentTestFunction());

    text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void SnapshotTests::test_SnapshotTesting_capture_QObject()
{
    QObject object;

    QString snapshot = SnapshotTesting::capture(&object);

    QCOMPARE(snapshot, QString(""));
}

void SnapshotTests::test_SnapshotTesting_matchStoredSnapshot()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;
    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void SnapshotTests::test_SnapshotTesting_matchStoredSnapshot_data()
{
    scanSamples();
}

void SnapshotTests::test_SnapshotTesting_matchStoredSnapshot_expandAll()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;

    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    SnapshotTesting::Options options;
    options.expandAll = true;
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem, options);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void SnapshotTests::test_SnapshotTesting_matchStoredSnapshot_expandAll_data()
{
    scanSamples();
}

void SnapshotTests::test_SnapshotTesting_matchStoredSnapshot_hideId()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;

    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    SnapshotTesting::Options options;
    options.hideId = true;
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem, options);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void SnapshotTests::test_SnapshotTesting_matchStoredSnapshot_hideId_data()
{
    scanSamples();
}

void SnapshotTests::scanSamples()
{
    QTest::addColumn<QString>("input");

    QStringList files = QtShell::find(QtShell::realpath_strip(SRCDIR, "sample"), "*.qml");

    foreach (QString file, files) {
        QTest::newRow(file.toUtf8().constData()) << file;
    }
}
