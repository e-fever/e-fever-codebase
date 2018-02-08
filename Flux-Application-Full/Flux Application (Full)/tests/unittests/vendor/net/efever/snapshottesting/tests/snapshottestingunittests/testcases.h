#ifndef SNAPSHOTTESTS_H
#define SNAPSHOTTESTS_H

#include <QObject>

class Testcases : public QObject
{
    Q_OBJECT
public:
    explicit Testcases(QObject *parent = nullptr);

signals:

public slots:

private slots:
    /* Private API */

    void init();

    void test_obtainQmlPackage();

    void test_obtainDynamicDefaultValues();

    void test_classNameToComponentName();

    void test_context();

    void test_loading_config();

    void test_grabImage();

    void test_ScreenshotBrowser();

    void test_convertToPackageNotation();

    void test_replaceLines();

    /* Public API */

    void test_SnapshotTesting_diff();

    void test_SnapshotTesting_saveSnapshots();

    void test_SnapshotTesting_addClassIgnoredProperty();

    void test_SnapshotTesting_capture_QObject();

    void test_SnapshotTesting_capture_RadioButton();

    void test_SnapshotTesting_matchStoredSnapshot();
    void test_SnapshotTesting_matchStoredSnapshot_data();

    void test_SnapshotTesting_matchStoredSnapshot_expandAll();
    void test_SnapshotTesting_matchStoredSnapshot_expandAll_data();

    void test_SnapshotTesting_matchStoredSnapshot_hideId();
    void test_SnapshotTesting_matchStoredSnapshot_hideId_data();

    void test_SnapshotTesting_matchStoredSnapshot_screenshot();
    void test_SnapshotTesting_matchStoredSnapshot_screenshot_data();

    void test_SnapshotTesting_createTest();
    void test_SnapshotTesting_createTest_data();

private:
    void scanSamples();
};


#endif // SNAPSHOTTESTS_H
