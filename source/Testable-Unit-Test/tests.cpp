#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include "tests.h"

Tests::Tests(QObject *parent) : QObject(parent)
{

    // This function do nothing but could make Qt Creator Autotests plugin recognize this unit test
    auto ref =[=]() {
        QTest::qExec(this, 0, 0);
    };
    Q_UNUSED(ref);

}

void Tests::testCase()
{

}

