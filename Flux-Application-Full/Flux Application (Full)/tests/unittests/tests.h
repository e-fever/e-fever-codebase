#pragma once
#include <QObject>

class Tests : public QObject
{
    Q_OBJECT
public:
    explicit Tests(QObject *parent = 0);

private slots:
    void test_qml_loading();
    void test_qml_loading_data();

    /* Snapshot Testing
     *
     * https://medium.com/e-fever/qml-snapshot-testing-with-tdd-aba81441c52
     */

    void test_Snapshot();
    void test_Snapshot_data();
};

