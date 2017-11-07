#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include <QtShell>
#include <QQuickWindow>
#include <snapshottesting.h>
#include "tests.h"

Tests::Tests(QObject *parent) : QObject(parent)
{
    auto ref = [=]() {
        QTest::qExec(this, 0, 0); // Autotest detect available test cases of a QObject by looking for "QTest::qExec" in source code
    };
    Q_UNUSED(ref);
}

void Tests::test_qml_loading()
{    
    QFETCH(QString, input);

    QQmlEngine engine;
    engine.addImportPath("qrc:///");


    QQmlComponent comp(&engine);
    comp.loadUrl(QUrl(input));

    if (comp.isError()) {
        qDebug() << QString("%1 : Load Failed. Reason :  %2").arg(input).arg(comp.errorString());
    }
    QVERIFY(!comp.isError());
}

void Tests::test_qml_loading_data()
{
    QTest::addColumn<QString>("input");
    QStringList files;
    files << QtShell::find(QtShell::realpath_strip(SRCDIR,"../../app/MYPROJECT/MYPACKAGE"), "*.qml");

    foreach (QString file , files) {
        QString content = QtShell::cat(file);
        content = content.toLower();

        if (content.indexOf("pragma singleton") != -1) {
            continue;
        }

        QTest::newRow(QtShell::basename(file).toLocal8Bit().constData()) << file;
    }
}


void Tests::test_Snapshot()
{
    QFETCH(QString, input);

    QUrl url = QUrl::fromLocalFile(QtShell::realpath_strip(input));
    qDebug() << input << url;

    QQmlEngine engine;
    engine.addImportPath("qrc:///");

    QQmlComponent comp(&engine);
    comp.loadUrl(url);

    QObject * object = comp.create();

    QString text = SnapshotTesting::capture(object);

    QString name = QString("%1/%2").arg(QTest::currentTestFunction()).arg(QtShell::basename(input));

    QQuickItem* item = qobject_cast<QQuickItem*>(object);
    QQuickWindow window;

    if (item) {

        window.setWidth(640);
        window.setHeight(480);
        window.setGeometry(0,0,640,480);
        window.show();
        item->setParentItem(window.contentItem());
    }

    SnapshotTesting::matchStoredSnapshot(name, text);

    delete object;
}

void Tests::test_Snapshot_data()
{
    QTest::addColumn<QString>("input");
    QStringList files;
    files << QtShell::find(QtShell::realpath_strip(SRCDIR,"../../app/MYPROJECT/MYPACKAGE"), "*.qml");

    foreach (QString file , files) {
        QString content = QtShell::cat(file);
        content = content.toLower();

        if (content.indexOf("pragma singleton") != -1) {
            continue;
        }

        QTest::newRow(QtShell::basename(file).toLocal8Bit().constData()) << file;
    }
}
