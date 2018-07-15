#pragma once
#include <QObject>

class TestCases : public QObject
{
    Q_OBJECT
public:
    explicit TestCases(QObject *parent = 0);

private slots:
    void test_qml_loading();
    void test_qml_loading_data();
};

