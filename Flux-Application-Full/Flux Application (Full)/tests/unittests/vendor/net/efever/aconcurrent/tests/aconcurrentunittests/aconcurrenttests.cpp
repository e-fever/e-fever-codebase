#include <QQmlApplicationEngine>
#include <QTest>
#include <Automator>
#include <QFutureWatcher>
#include <aconcurrent.h>
#include "aconcurrenttests.h"

//@TODO - Migrate to Automator::waitUntil()

void tick() {
    for (int i = 0 ; i < 3;i++) {
        Automator::wait(10);
    }
}

template <typename F>
bool waitUntil(F f, int timeout = -1) {
    QTime time;
    time.start();

    while (!f()) {
        Automator::wait(10);
        if (timeout > 0 && time.elapsed() > timeout) {
            tick();
            return false;
        }
    }
    tick();
    return true;
}

template <typename T>
bool waitUntil(QFuture<T> future, int timeout = -1) {
    return waitUntil([=]() {
       return future.isFinished();
    }, timeout);
    tick();
}

bool inMainThread() {
    return QThread::currentThread() == QCoreApplication::instance()->thread();
}

using namespace AConcurrent;

AConcurrentTests::AConcurrentTests(QObject *parent) : QObject(parent)
{
    auto ref = [=]() {
        QTest::qExec(this, 0, 0); // Autotest detect available test cases of a QObject by looking for "QTest::qExec" in source code
    };
    Q_UNUSED(ref);
    pool.setMaxThreadCount(4);
}

void AConcurrentTests::test_mapped()
{
    auto worker = [](int value) {
        return value * value;
    };

    int count = 200;
    QList<int> input;
    QList<int> expected;

    for (int i = 0 ; i < count ; i++) {
        input << (i+1);
        expected << (i+1) * (i+1);
    }

    QFuture<int> future = AConcurrent::mapped(&pool, input, worker);

    QCOMPARE(future.progressMaximum(), count);
    QCOMPARE(future.isFinished(), false);
    AConcurrent::await(future);

    QVERIFY(future.isFinished());
    QCOMPARE(future.resultCount(), count);

    QList<int> result;
    result = future.results();

    QVERIFY(result == expected);
}

void AConcurrentTests::test_mapped_void()
{
    int count = 0;
    QMutex mutex;

    auto worker = [&](int value) -> void {
        Q_UNUSED(value);
        mutex.lock();
        count++;
        mutex.unlock();
    };

    QList<int> input;

    for (int i = 0 ; i < 3 ; i++) {
        input << (i+1);
    }

    QFuture<void> future = AConcurrent::mapped(&pool, input, worker);
    AConcurrent::await(future);

    QCOMPARE(count, 3);
}

void AConcurrentTests::test_mapped_memory()
{
    static int count = 0;

    class Dummy {
    public:

        Dummy() {
            count++;
        }

        ~Dummy() {
            count--;
        }
    };

    QSharedPointer<Dummy> dummy = QSharedPointer<Dummy>::create();
    QCOMPARE(count, 1);

    class Data {
    public:
        int result;
        QSharedPointer<Dummy> ref;
    };

    auto worker = [&](int value) -> Data {
        Data data;
        data.result = value * value;
        data.ref = dummy;
        return data;
    };

    {
        QList<int> input;
        input << 1 << 2 << 3;

        QFuture<Data> future = AConcurrent::mapped(&pool, input, worker);

        AConcurrent::await(future);

        QVERIFY(future.isFinished());
        QCOMPARE(count , 1);

        dummy.clear();
        QCOMPARE(count , 1); // The reference is not cleared yet.

        QList<Data> results = future.results();
        QCOMPARE(results.size(), 3);
        QCOMPARE(results[0].result, 1);
        QCOMPARE(results[1].result, 4);
        QCOMPARE(results[2].result, 9);
    }

    Automator::wait(10);
    QCOMPARE(count , 0);
}

void AConcurrentTests::test_mapped_in_non_main_thread()
{
    auto thread = [=]() {

        auto worker = [](int value) {
            return value * value;
        };

        int count = 200;
        QList<int> input;
        QList<int> expected;

        for (int i = 0 ; i < count ; i++) {
            input << (i+1);
            expected << (i+1) * (i+1);
        }

        QFuture<int> future = AConcurrent::mapped(&pool, input, worker);
        AConcurrent::await(future);

        QVERIFY(future.isFinished());

        QList<int> result;
        result = future.results();
        QVERIFY(result == expected);
    };

    AConcurrent::await(QtConcurrent::run(&pool, thread));
}

void AConcurrentTests::test_mapped_progress()
{

    auto worker = [&](int value) -> int {
        Automator::wait(50);
        return value * value;
    };

    int count = 50;
    QList<int> input;
    QList<int> expected;
    QList<int> progressList;

    for (int i = 0 ; i < count ; i++) {
        input << (i+1);
        expected << (i+1) * (i+1);
    }

    QFuture<int> future = AConcurrent::mapped(&pool, input, worker);
    QCOMPARE(future.progressValue(), 0);

    QFutureWatcher<int> watcher;

    connect(&watcher, &QFutureWatcher<int>::progressValueChanged, [&] (int value) {
        progressList << value;
    });
    watcher.setFuture(future);

    AConcurrent::await(future);
    Automator::wait(50);
    QCOMPARE(future.results().size(), count);
    QCOMPARE(future.progressValue(), count);

    QCOMPARE(progressList.last(), count);
    for (int i = 0 ; i < progressList.size() - 1;i++) {
        QVERIFY(progressList[i] < progressList[i+1]);
    }


}

void AConcurrentTests::test_blockingMapped()
{
    auto worker = [](int value) {
        return value * value;
    };

    QList<int> input;
    input << 1 << 2 << 3;

    QList<int> result = AConcurrent::blockingMapped(QThreadPool::globalInstance(), input, worker);
    QList<int> expected;
    expected  << 1 << 4 << 9;

    QVERIFY(result == expected);
}

void AConcurrentTests::test_queue()
{
    int count = 0;
    auto worker = [&](int value) -> qreal {
        Automator::wait(50);
        count++;
        return value * value;
    };

    auto queue = AConcurrent::queue(QThreadPool::globalInstance(), worker);
    QCOMPARE(queue.count(), 0);
    QCOMPARE(count, 0);

    auto f = queue.future();
    QCOMPARE(f.isFinished(), false);

    queue.enqueue(2);
    QCOMPARE(queue.head(), 2);
    QCOMPARE(queue.count(), 1);
    QCOMPARE(f.isFinished(), false);

    queue.run();
    QCOMPARE(f.isFinished(), false);

    waitUntil([=]() {
       return f.isFinished();
    }, 1000);

    QCOMPARE(f.isFinished(), true);
    QCOMPARE(f.result(), 4.0);
    QCOMPARE(queue.count(), 1);
    QCOMPARE(count, 1);

    queue.enqueue(3);
    QCOMPARE(f.isFinished(), true);
    QCOMPARE(queue.count(), 2);

    queue.dequeue();
    QCOMPARE(queue.count(), 1);

    f = queue.future();
    queue.run();
    waitUntil([=]() {
       return f.isFinished();
    }, 1000);

    QCOMPARE(f.isFinished(), true);
    QCOMPARE(f.result(), 9.0);
    QCOMPARE(queue.count(), 1);
    QCOMPARE(count, 2);
}

void AConcurrentTests::test_runOnMainThread()
{
    {
        // runOnMainThread<void> in main thread
        int count = 0;
        QFuture<void> future = AConcurrent::runOnMainThread([&]() {
            count++;
            QVERIFY(inMainThread());
        });
        QCOMPARE(count, 0);
        AConcurrent::await(future);
        QCOMPARE(count, 1);

    }

    {
        bool valid = false;
        // runOnMainThread<int> in main thread
        QFuture<int> future = AConcurrent::runOnMainThread([&]() {
            valid = inMainThread();
            return 9;
        });
        AConcurrent::await(future);
        QCOMPARE(future.result(), 9);
        QCOMPARE(valid, true);
    }

    {
        int result = 0;
        bool valid = false;
        auto worker = [&]() {
            QFuture<int> future = AConcurrent::runOnMainThread([&]() {
                valid = inMainThread();
                return 9;
            });
            AConcurrent::await(future);
            QCOMPARE(future.result(), 9);
            result = future.result() + 1;
        };
        AConcurrent::await(QtConcurrent::run(worker));
        QCOMPARE(result, 10);
    }

}

void AConcurrentTests::test_debounce()
{
    {
        QString k1 = AConcurrent::Private::key(this, "key1");
        QString k2 = AConcurrent::Private::key(this, "key2");

        QVERIFY(k1 != k2);
        qDebug() << k1 << k2;

    }


    auto worker = [&](int value) {
        Automator::wait(50);
        return value * value;
    };

    int count = 0;

    auto cleanup = [&]() {
        count++;
    };

    {
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 0);
        auto f1 = QtConcurrent::run(worker, 2);
        AConcurrent::debounce(this, "single", f1, cleanup);
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 1);
        await(f1);
        Automator::wait(50);

        QCOMPARE(count , 1);
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 0);
    }

    {
        QList<int> seq;
        // Calling twice, only the last function will be executed.
        QString key = "double";
        auto f1 = QtConcurrent::run(worker, 3);
        AConcurrent::debounce(this, key, f1, [&]() {
            seq << 1;
            cleanup();
        });

        QCOMPARE(AConcurrent::Private::debounceStore.size(), 1);

        auto f2 = QtConcurrent::run(worker, 4);
        AConcurrent::debounce(this, key, f2, [&]() {
            seq << 2;
            cleanup();
        });
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 1);

        auto combined = AsyncFuture::combine();
        combined << f1 << f2;
        await(combined.future());

        QCOMPARE(count , 2);
        Automator::wait(50);
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 0);
        QCOMPARE(seq.size(), 1);
        QCOMPARE(seq[0], 2);
    }

    {
        // Verify cancel
        QString key = "cancel";
        auto defer = AsyncFuture::deferred<void>();
        AConcurrent::debounce(this, key, defer.future(), cleanup);
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 1);
        defer.cancel();
        Automator::wait(50);
        QCOMPARE(AConcurrent::Private::debounceStore.size(), 0);
        QCOMPARE(count , 2);
    }


}

void AConcurrentTests::test_pipeline()
{
    auto worker = [&](int value) -> qreal {
        return value * value;
    };

    {

        QFuture<qreal> future;

        {
            auto pipeline = AConcurrent::pipeline(&pool, worker);
            pipeline.add(0);
            pipeline.close();

            future = pipeline.future();
            QCOMPARE(future.isFinished(), false);
        }
        QCOMPARE(future.isFinished(), false);

        AConcurrent::await(future);
        QCOMPARE(future.isFinished(), true);
    }

    {
        QFuture<qreal> future;

        {
            AConcurrent::Pipeline<qreal,int> pipeline;
            pipeline = AConcurrent::pipeline(&pool, worker);
            pipeline.add(0);
            pipeline.close();

            future = pipeline.future();
            QCOMPARE(future.isFinished(), false);

        }

        QCOMPARE(future.isFinished(), false);

        AConcurrent::await(future);
        QCOMPARE(future.isFinished(), true);
    }


}

void AConcurrentTests::test_pipeline_close()
{
    int count = 0;
    auto worker = [&](int value) -> qreal {
        Automator::wait(50);
        count++;
        return value * value;
    };

    QThreadPool pool;
    pool.setMaxThreadCount(2);

    auto pipeline = AConcurrent::pipeline(&pool, worker);

    QCOMPARE(pipeline.future().isFinished(), false);
    auto result = pipeline.future();
    QCOMPARE(result.progressValue(), 0);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 0);
    QCOMPARE(result.isFinished(), false);

    auto future = pipeline.add(0);

    AConcurrent::await(future);
    Automator::wait(100);

    QCOMPARE(future.isFinished(), true);
    QCOMPARE(future.result(), 0.0);
    QCOMPARE(result.resultCount(), 1);

    QCOMPARE(result.progressValue(), 1);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 1);
    QCOMPARE(result.isFinished(), false);

    future = pipeline.add(1);
    AConcurrent::await(future);

    QCOMPARE(future.isFinished(), true);
    QCOMPARE(future.result(), 1.0);

    QCOMPARE(result.progressValue(), 2);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 2);
    QCOMPARE(result.isFinished(), false);

    QCOMPARE(result.resultCount(), 2);

    QCOMPARE(result.resultAt(0), 0.0);
    QCOMPARE(result.resultAt(1), 1.0);

    /// Complete the pipeline. No more tasks can be added.
    pipeline.add(2);
    pipeline.add(3);

    pipeline.close();
    Automator::wait(200);
    QCOMPARE(result.isFinished(), true);
    QCOMPARE(result.progressValue(), 4);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 4);

    QCOMPARE(result.resultCount(), 4);

    QCOMPARE(result.results().size(), 4);

    future = pipeline.add(2);
    Automator::wait(10);
    QCOMPARE(future.isFinished(), true);
    QCOMPARE(future.isCanceled(), true);

}

void AConcurrentTests::test_pipeline_close_after_finished()
{
    int count = 0;
    QMutex mutex;

    auto worker = [&](int value) -> qreal {
        mutex.lock();
        count++;
        mutex.unlock();
        return value * value;
    };

    QThreadPool pool;
    pool.setMaxThreadCount(2);

    auto pipeline = AConcurrent::pipeline(&pool, worker);

    auto result = pipeline.future();
    QList<QFuture<qreal>> futures;

    for (int i = 0 ; i < 6;i++) {
        futures << pipeline.add(i);
    }

    Automator::wait(100);
    QCOMPARE(count, 6);
    QCOMPARE(result.isFinished(), false);

    pipeline.close();
    Automator::wait(100);
    QCOMPARE(result.isFinished(), true);
    QCOMPARE(result.results().size(), 6);

}

void AConcurrentTests::test_pipeline_close_after_added()
{
    QSemaphore semaphore(6);
    semaphore.acquire(6);
    int count = 0;
    QMutex mutex;

    auto worker = [&](int value) -> qreal {
        semaphore.acquire(1);
        mutex.lock();
        count++;
        mutex.unlock();
        return value * value;
    };

    QThreadPool pool;
    pool.setMaxThreadCount(2);

    auto pipeline = AConcurrent::pipeline(&pool, worker);
    QList<qreal> expected;
    for (int i = 0 ; i < 6; i++) {
        pipeline.add(i);
        expected << i * i;
    }
    pipeline.close();

    semaphore.release(6);

    AConcurrent::await(pipeline.future());

    QList<qreal> results = pipeline.future().results();

    QVERIFY(expected == results);
}

void AConcurrentTests::test_pipeline_cancel()
{
    int count = 0;
    QMutex mutex;
    QSemaphore semaphore(6);
    semaphore.acquire(6);

    auto worker = [&](int value) -> qreal {
        semaphore.acquire();
        mutex.lock();
        count++;
        mutex.unlock();
        return value * value;
    };

    QThreadPool pool;
    pool.setMaxThreadCount(2);

    {
    auto pipeline = AConcurrent::pipeline(&pool, worker);

    auto result = pipeline.future();
    QList<QFuture<qreal>> futures;

    for (int i = 0 ; i < 6;i++) {
        futures << pipeline.add(i);
    }
    Automator::wait(10);

    QCOMPARE(result.progressValue(), 0);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 6);

    semaphore.release(2);
    Automator::wait(10);

    QCOMPARE(result.progressValue(), 2);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 6);

    result.cancel();

    auto future = pipeline.add(5);
    Automator::wait(10);
    QCOMPARE(future.isFinished(), true);
    QCOMPARE(future.isCanceled(), true);

    semaphore.release(4);

    Automator::wait(100);

    QCOMPARE(result.progressValue(), 2);
    QCOMPARE(result.progressMinimum(), 0);
    QCOMPARE(result.progressMaximum(), 6);

    QCOMPARE(futures[0].isFinished(), true);
    QCOMPARE(futures[1].isFinished(), true);
    QCOMPARE(futures[2].isFinished(), true);
    QCOMPARE(futures[3].isFinished(), true);
    QCOMPARE(futures[4].isCanceled(), true);
    QCOMPARE(futures[5].isCanceled(), true);
    QCOMPARE(count, 4);
    }
    Automator::wait(100);
}

void AConcurrentTests::test_pipeline_void()
{
    int count = 0;

    QMutex mutex;
    auto worker = [&](int value) -> void {
        Q_UNUSED(value);
        mutex.lock();
        count++;
        mutex.unlock();
    };

    QThreadPool pool;
    pool.setMaxThreadCount(2);

    {
        auto pipeline = AConcurrent::pipeline(&pool, worker);

        for (int i = 0 ; i < 6;i++) {
            pipeline.add(i);
        }
        pipeline.close();

        QCOMPARE(pipeline.future().isFinished(), false);
        AConcurrent::await(pipeline.future());
        QCOMPARE(pipeline.future().isFinished(), true);

        QCOMPARE(count, 6);
    }
    // Execute event loop and make sure the memory allocated for pipeline is released.
    Automator::wait(100);
}

void AConcurrentTests::test_pipeline_constructor()
{
    {
        auto worker = [](int value) {
            Q_UNUSED(value);
        };

        QList<int> input;
        input << 0 << 1 << 2;

        QFuture<void> future;

        {
            auto pipeline = AConcurrent::pipeline(&pool , worker, input);
            pipeline.close();
            future = pipeline.future();
        }

        AConcurrent::await(future);
        QCOMPARE(future.progressMaximum(), 3);
        QCOMPARE(future.progressValue(), 3);
    }

}


void AConcurrentTests::test_pipeline_dynamic_add()
{
    class Session {
    public:
        AConcurrent::Pipeline<void, bool> pipeline;
    };

    Session* session = new Session();

    auto worker = [=](bool value) {
        Automator::wait(50);
        if (value) {
            Automator::wait(50);
            session->pipeline.add(false);
        }
    };

    QList<bool> input;
    input << false << false << false << true;

    session->pipeline = AConcurrent::pipeline(&pool, worker, input);
    auto future = session->pipeline.future();

    AsyncFuture::observe(future).progress([=]() mutable {
        qDebug() << future.progressValue() << future.progressMaximum();
        if (future.progressValue() == future.progressMaximum()) {
            session->pipeline.close();
            delete session;
        }
    });

    AConcurrent::await(future);
    qDebug() << "Done";

    QCOMPARE(future.progressValue(), 5);

    QCOMPARE(future.progressMaximum(), 5);

}

