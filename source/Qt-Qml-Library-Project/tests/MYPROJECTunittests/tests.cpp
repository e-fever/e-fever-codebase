#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include <QtShell>
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
    files << QtShell::find(QtShell::realpath_strip(SRCDIR,"../../MYPACKAGE"), "*.qml");

    foreach (QString file , files) {
        QString content = QtShell::cat(file);
        content = content.toLower();

        if (content.indexOf("pragma singleton") != -1) {
            continue;
        }

        QTest::newRow(QtShell::basename(file).toLocal8Bit().constData()) << file;
    }
}

