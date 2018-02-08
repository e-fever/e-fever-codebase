#include <QTest>
#include <QtShell>
#include <QQmlApplicationEngine>
#include <QQmlComponent>
#include <QQuickItem>
#include <QJsonDocument>
#include <QQuickItemGrabResult>
#include <QQuickWindow>
#include <QMetaObject>
#include <aconcurrent.h>
#include <private/qqmldata_p.h>
#include <private/qqmlcontext_p.h>
#include "automator.h"
#include "testcases.h"
#include "testablefunctions.h"
#include "snapshottesting.h"
#include "private/snapshottesting_p.h"

using namespace QtShell;
using namespace SnapshotTesting;
using namespace SnapshotTesting::Private;

Testcases::Testcases(QObject *parent) : QObject(parent)
{
    auto ref = [=]() {
        QTest::qExec(this, 0, 0); // Autotest detect available test cases of a QObject by looking for "QTest::qExec" in source code
    };
    Q_UNUSED(ref);
}

void Testcases::init()
{
    {
        // Make sure the QtQuick package is loaded

        QQmlEngine engine;
        createQmlComponent(&engine, "Item", "QtQuick", 2 , 0)->deleteLater();
    }
}

void Testcases::test_obtainQmlPackage()
{
    QQuickItem* item = new QQuickItem();
    QString package = obtainQmlPackage(item);

    QCOMPARE(package, QString("QtQuick"));
    delete item;
}

void Testcases::test_obtainDynamicDefaultValues()
{
    QQuickItem* item = new QQuickItem();

    item->setX(100);
    QVariantMap defaultValues = obtainDynamicDefaultValues(item);

    QVERIFY(defaultValues.contains("x"));
    QCOMPARE(defaultValues["x"].toInt(), 0);

    delete item;

}

void Testcases::test_classNameToComponentName()
{
    QCOMPARE(classNameToComponentName("AnyOtherClass"), QString("AnyOtherClass"));
    QCOMPARE(classNameToComponentName("AnyOtherClass_QML_123"), QString("AnyOtherClass"));
    QCOMPARE(classNameToComponentName("QQuickItem"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickItem_QML_123"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickItem_QML_4523"), QString("Item"));
    QCOMPARE(classNameToComponentName("QQuickText"), QString("Text"));

    {
        QQmlEngine engine;
        QObject* object = createQmlComponent(&engine, "Canvas", "QtQuick", 2, 0);

        QCOMPARE(classNameToComponentName(object->metaObject()->className()), QString("Canvas"));
        object->deleteLater();
    }

}

void Testcases::test_context()
{
    QQmlApplicationEngine engine;

    auto componentNameByBaseContext = [](QObject * object) {
        QQmlContext* context = obtainBaseContext(object);
        QString res;
        if (context) {
            res = obtainComponentNameByBaseUrl(context->baseUrl());
        }

        return res;
    };

    {
        // Button
        QObject* object = createQmlComponent(&engine, "Button", "QtQuick.Controls", 2, 0);
        QVERIFY(object);
        QCOMPARE(listContextUrls(object).size(), 1);

        QQmlContext* context = qmlContext(object);
        QVERIFY(context);
        QVERIFY(context->contextObject());

        object->deleteLater();
    }

    {
        QObject* object = createQmlComponent(&engine, "Item", "QtQuick", 2, 0);
        QVERIFY(object);
        QVERIFY(listContextUrls(object).size()  == 0);
        object->deleteLater();
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/CustomButton.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QCOMPARE(listContextUrls(object).size(), 2);

        QCOMPARE(obtainComponentNameOfQtType(object), QString("Button"));
        qDebug() << "CustomButton" << listContextUrls(object);

        object->deleteLater();
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample1.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QQmlContext* context = qmlContext(object);

        QCOMPARE(obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample1"));

        context = SnapshotTesting::Private::obtainCreationContext(object);

        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample1"));

        QCOMPARE(SnapshotTesting::Private::obtainSourceComponentName(object), QString("Item"));

        QVERIFY(obtainCurrentScopeContext(object) == qmlContext(object));

        {
            QQuickItem* item = object->findChild<QQuickItem*>("Item10");
            QVERIFY(item);

            QCOMPARE(SnapshotTesting::Private::obtainSourceComponentName(item), QString("Column"));
        }
        delete object;
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample2.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        {
            QObject* innerItem = object->findChild<QQuickItem*>("InnerItem");
            QVERIFY(innerItem);

            QCOMPARE(obtainComponentNameByCurrentScopeContext(innerItem), QString("Item"));

            // The component don't have it's own scope
            QCOMPARE(componentNameByBaseContext(innerItem), QString(""));

        }

        {
            QObject* item = object->findChild<QQuickItem*>("item_sample1");
            QVERIFY(item);

            QCOMPARE(obtainComponentNameByCurrentScopeContext(item), QString("Sample1"));

            QCOMPARE(componentNameByBaseContext(item), QString("Sample1"));
        }
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample3.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        {
            QObject* item = object->findChild<QQuickItem*>("ListView");
            QVERIFY(item);

            QObjectList list = obtainChildrenObjectList(item);

            QVERIFY(list.size() > 0);
            QObject* child = list.first();

            QCOMPARE(obtainComponentNameByCurrentScopeContext(child), QString("Item"));
        }

        {
            QObject* item = object->findChild<QQuickItem*>("Repeater");
            QVERIFY(item);

            QQuickItem* child;
            QMetaObject::invokeMethod(item,"itemAt",Qt::DirectConnection,
                                      Q_RETURN_ARG(QQuickItem*, child),
                                      Q_ARG(int,0));

            // obtainComponentNameByCurrentScopeContext can not obtain the current component name in this case
            QCOMPARE(obtainComponentNameByCurrentScopeContext(child), QString("Sample3")); // It should be "Item"

            QCOMPARE(componentNameByBaseContext(child), QString(""));

        }

        {
            QObject* innerItem = object->findChild<QQuickItem*>("InnerItem");
            QVERIFY(innerItem);

            QCOMPARE(obtainComponentNameByCurrentScopeContext(innerItem), QString("Item"));
        }

    }


    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample5.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);
        QQmlContext* context = qmlContext(object);
        QCOMPARE(obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample5"));

        context = SnapshotTesting::Private::obtainCreationContext(object);
        QCOMPARE(obtainComponentNameByBaseUrl(context->baseUrl()), QString("Sample5Form"));

        QVERIFY(obtainCurrentScopeContext(object) == qmlContext(object));
        QCOMPARE(SnapshotTesting::Private::obtainSourceComponentName(object), QString("Sample5Form"));

    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample2.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QQuickItem* child = object->findChild<QQuickItem*>("item_sample1");

        QVERIFY(obtainCurrentScopeContext(child) != qmlContext(child));
        QCOMPARE(obtainSourceComponentName(object), QString("Item"));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample6.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(SnapshotTesting::Private::obtainComponentNameByCreationContext(object), QString("Sample5Form"));

        QCOMPARE(obtainComponentNameByBaseUrl(obtainCurrentScopeContext(object)->baseUrl()), QString("Sample6"));

        QCOMPARE(SnapshotTesting::Private::obtainSourceComponentName(object), QString("Sample5"));
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample7.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(obtainComponentNameByClass(object), QString("Sample7"));

        QCOMPARE(SnapshotTesting::Private::obtainComponentNameOfQtType(object), QString("Item"));
        QCOMPARE(SnapshotTesting::Private::obtainSourceComponentName(object), QString("Item"));
        QCOMPARE(SnapshotTesting::Private::obtainSourceComponentName(object, true), QString("Item"));

        delete object;
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample_Control1.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        {
            QQuickItem *radioButton = qobject_cast<QQuickItem*>(object->findChild<QQuickItem*>("radioButton"));
            QVERIFY(radioButton);

            QCOMPARE(componentNameByBaseContext(radioButton), QString("RadioButton"));
        }
    }

    {
        QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(SRCDIR, "sample/Sample_Layout.qml"));

        QQmlComponent component(&engine,url);

        QQuickItem *object = qobject_cast<QQuickItem*>(component.create());
        QVERIFY(object);

        QCOMPARE(obtainComponentNameOfQtType(object), QString("Grid"));

    }
}

void Testcases::test_loading_config()
{
    {
        QString text = QtShell::cat(":/qt-project.org/imports/SnapshotTesting/config/snapshot-config.json");

        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(),&error);
        Q_UNUSED(doc);

        QVERIFY(error.error == QJsonParseError::NoError);
    }
}

void Testcases::test_grabImage()
{
    if (Testable::isCI()) {
        qDebug() << "Skip this test in CI environment";
        return;
    }

    QQuickWindow window;
    QQmlEngine engine;
    QObject* object = createQmlComponent(&engine, "Item", "QtQuick", 2, 0);

    QQuickItem *item = qobject_cast<QQuickItem*>(object);
    QVERIFY(item);
    item->setWidth(200);
    item->setHeight(300);

    item->setParentItem(window.contentItem());
    window.show();

    Automator::wait(2000);

    QFuture<QImage> future = grabImage(item);

    AConcurrent::await(future);

    QImage image = future.result();

    QCOMPARE(image.size(), QSize(200,300));
}

void Testcases::test_ScreenshotBrowser()
{
    QImage image1(QSize(320,240), QImage::Format_RGB32);
    image1.fill(QColor(255,0,0));

    QImage image2(QSize(160,240), QImage::Format_RGB32);
    image2.fill(QColor(0,0,255));

    QString function = QTest::currentTestFunction();

    QByteArray base64 = SnapshotTesting::Private::toBase64(image1);

    QQmlApplicationEngine engine;

    QQuickWindow window;

    QQmlComponent component(&engine, QUrl("qrc:///qt-project.org/imports/SnapshotTesting/ScreenshotBrowser.qml"));

    QQuickItem* item = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(item);

    item->setSize(QSize(640,480));
    item->setParentItem(window.contentItem());
    item->setProperty("screenshot", base64);

    window.resize(QSize(item->width(), item->height()));
    window.show();

    auto replace = [=](const QString& source) {
        QString t = source;
        t = t.replace(QRegExp("source: \\"[a-zA-Z0-9=+;:,/]*\\""),"");
        t = t.replace(QRegExp("screenshot: \\"[a-zA-Z0-9=+;:,/]*\\""),"");
        t = t.replace(QRegExp("[a-z]*Screenshot: \\"[a-zA-Z0-9=+;:,/]*\\""),"");
        return t;
    };

    Automator::wait(100);

    SnapshotTesting::CaptureOptions options;
    options.expandAll = true;

    {
        QString snapshot = SnapshotTesting::capture(item, options);
        snapshot = replace(snapshot);

        QVERIFY(SnapshotTesting::matchStoredSnapshot(function + "_Single", snapshot));
    }

    {
        item->setProperty("previousScreenshot", SnapshotTesting::Private::toBase64(image2));
        Automator::wait(100);

        QString snapshot = SnapshotTesting::capture(item, options);
        snapshot = replace(snapshot);

        QVERIFY(SnapshotTesting::matchStoredSnapshot(function + "_Dual", snapshot));
    }

    {
        QImage combined = SnapshotTesting::Private::combineImages(image1, image2);

        item->setProperty("combinedScreenshot", SnapshotTesting::Private::toBase64(combined));
        QMetaObject::invokeMethod(item, "showCombinedScreenshot");

        Automator automator(&engine);
        Automator::wait(10);

        QString snapshot = SnapshotTesting::capture(item, options);
        snapshot = replace(snapshot);

        QVERIFY(SnapshotTesting::matchStoredSnapshot(function + "_Combined", snapshot));

    }



    delete item;
}

void Testcases::test_convertToPackageNotation()
{
    {
        QUrl url("qrc:///qt-project.org/imports/SnapshotTesting/ScreenshotBrowser.qml");

        QCOMPARE(converToPackageNotation(url), QString("qt-project.org.imports.SnapshotTesting"));
    }

    {
        QUrl url("qrc:///qt-project.org/imports/SnapshotTesting.2/ScreenshotBrowser.qml");

        QCOMPARE(converToPackageNotation(url), QString("qt-project.org.imports.SnapshotTesting"));
    }

}

void Testcases::test_replaceLines()
{
    QString input = "123\\n456\\n789";
    QString expectedOutput = "123\\n\\n789";

    QCOMPARE(SnapshotTesting::replaceLines(input, QRegExp(".*5.*"),""), expectedOutput);
}

void Testcases::test_SnapshotTesting_diff()
{
    QString text1 = "A\\nB\\nC";
    QString text2 = "A\\nD\\nC";

    QString result = SnapshotTesting::diff(text1, text2);

    qDebug().noquote() << result;
}

void Testcases::test_SnapshotTesting_saveSnapshots()
{
    SnapshotTesting::saveSnapshots();
}

void Testcases::test_SnapshotTesting_addClassIgnoredProperty()
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

void Testcases::test_SnapshotTesting_capture_QObject()
{
    QObject object;

    QString snapshot = SnapshotTesting::capture(&object);

    QCOMPARE(snapshot, QString(""));
}

void Testcases::test_SnapshotTesting_capture_RadioButton()
{
    QQmlEngine engine;
    QString function = QTest::currentTestFunction();


    {
        QString qml = "import QtQuick 2.0; import QtQuick.Controls 1.1; Item { RadioButton {} }";

        QQmlComponent comp (&engine);
        comp.setData(qml.toUtf8(),QUrl());
        QObject* ret = comp.create();

        QQuickItem* item = qobject_cast<QQuickItem*>(ret);

        QVERIFY(item);

        SnapshotTesting::CaptureOptions options;
        options.expandAll = true;
        QString snapshot = SnapshotTesting::capture(item, options);
        qDebug() << snapshot;
        QVERIFY(SnapshotTesting::matchStoredSnapshot(function + "_Normal", snapshot));

        item->deleteLater();
    }

    {

        QObject* object = createQmlComponent(&engine, "RadioButton", "QtQuick.Controls",1,1);
        QQuickItem* item = qobject_cast<QQuickItem*>(object);
        QVERIFY(item);

        qDebug() << item << item->metaObject()->className();

        qDebug() << item->metaObject()->superClass()->superClass()->className();

        SnapshotTesting::CaptureOptions options;
        options.expandAll = true;
        QString snapshot = SnapshotTesting::capture(item, options);
        QVERIFY(SnapshotTesting::matchStoredSnapshot(function + "_Single", snapshot));

        item->deleteLater();

    }


}

void Testcases::test_SnapshotTesting_matchStoredSnapshot()
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

void Testcases::test_SnapshotTesting_matchStoredSnapshot_data()
{
    scanSamples();
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_expandAll()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;

    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    SnapshotTesting::CaptureOptions options;
    options.expandAll = true;
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem, options);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_expandAll_data()
{
    scanSamples();
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_hideId()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlApplicationEngine engine;

    QUrl url = QUrl::fromLocalFile(input);

    QQmlComponent component(&engine,url);
    QQuickItem *childItem = qobject_cast<QQuickItem*>(component.create());
    QVERIFY(childItem);

    SnapshotTesting::CaptureOptions options;
    options.hideId = true;
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QString text = SnapshotTesting::capture(childItem, options);
    text.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    text.replace(QString(SRCDIR), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, text));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_hideId_data()
{
    scanSamples();
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_screenshot()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);
    QString name = QString("%1_%2").arg(QTest::currentTestFunction()).arg(fileName);

    QQmlEngine engine;
    SnapshotTesting::Renderer renderer(&engine);

    SnapshotTesting::CaptureOptions options;
    options.hideId = true;

    QVERIFY(renderer.load(input));

    renderer.waitWhenStill(1000);

    QString snapshot = renderer.capture(options);
    QImage screenshot = renderer.grabScreenshot();;

    snapshot.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");

    QVERIFY(SnapshotTesting::matchStoredSnapshot(name, snapshot, screenshot));
}

void Testcases::test_SnapshotTesting_matchStoredSnapshot_screenshot_data()
{
    scanSamples();
}

void Testcases::test_SnapshotTesting_createTest()
{
    QFETCH(QString, input);

    QString fileName = QtShell::basename(input);

    QQmlEngine engine;
    Renderer renderer(&engine);
    QVERIFY(renderer.load(input));

    auto test = SnapshotTesting::createTest();

    QCOMPARE(test.name(), QString(QTest::currentTestFunction()));

    test.setSuffix(QString("_") + fileName);

    QString snapshot = test.capture(renderer.item());
    snapshot.replace(QUrl::fromLocalFile(QString(SRCDIR)).toString(), "");
    snapshot.replace(QString(SRCDIR), "");

    QImage screenshot = renderer.grabScreenshot();

    QVERIFY(test.match(snapshot, screenshot));
}

void Testcases::test_SnapshotTesting_createTest_data()
{
    scanSamples();
}

void Testcases::scanSamples()
{
    QTest::addColumn<QString>("input");

    QStringList files = QtShell::find(QtShell::realpath_strip(SRCDIR, "sample"), "*.qml");

    foreach (QString file, files) {
        QTest::newRow(file.toUtf8().constData()) << file;
    }
}
