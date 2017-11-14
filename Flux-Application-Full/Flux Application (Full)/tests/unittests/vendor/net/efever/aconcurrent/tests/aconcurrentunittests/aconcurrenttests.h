#pragma once
#include <QObject>
#include <QThreadPool>

class AConcurrentTests : public QObject
{
    Q_OBJECT
public:
    explicit AConcurrentTests(QObject *parent = 0);

private slots:

    void test_mapped();

    void test_mapped_void();

    void test_mapped_memory();

    void test_mapped_in_non_main_thread();

    void test_mapped_progress();

    void test_blockingMapped();

    void test_queue();

    void test_runOnMainThread();

    void test_debounce();

    void test_pipeline();

    void test_pipeline_close();

    void test_pipeline_close_after_finished();

    void test_pipeline_close_after_added();

    void test_pipeline_cancel();

    void test_pipeline_void();

    void test_pipeline_constructor();

    void test_pipeline_dynamic_add();

private:

    QThreadPool pool;
};

