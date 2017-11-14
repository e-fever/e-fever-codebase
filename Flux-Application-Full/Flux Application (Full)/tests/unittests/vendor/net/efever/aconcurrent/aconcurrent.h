#pragma once

#include <QFuture>
#include <QtConcurrent>
#include <QThreadPool>
#include <QTimer>
#include <asyncfuture.h>
#include <functional>

/* Enhance QtConcurrent by AsyncFuture
 *
 * 1) mapped() may take lambda function as input
 * 2) blockingMapped() - Run with event loop, don't block UI
 *
 */

namespace AConcurrent {

    namespace Private {

        template <typename Functor>
        inline void runOnMainThreadVoid(Functor func)  {
            QObject tmp;
            QObject::connect(&tmp, &QObject::destroyed, QCoreApplication::instance(), func, Qt::QueuedConnection);
        }

        // Value is a wrapper of data structure which could contain <void> type.
        template <typename R>
        class Value {
        public:
           Value() {
           }

           template <typename Functor>
           void run(Functor functor) {
               value = functor();
           }

           void complete(AsyncFuture::Deferred<R> defer) {
               defer.complete(value);
           }

           R value;
        };

        template <>
        class Value<void> {
        public:
            Value() {

            }

            template <typename Functor>
            void run(Functor functor) {
                functor();
            }

            void complete(AsyncFuture::Deferred<void> defer) {
                defer.complete();
            }
        };

        // function_traits: Source: http://stackoverflow.com/questions/7943525/is-it-possible-to-figure-out-the-parameter-type-and-return-type-of-a-lambda

        template <typename T>
        struct function_traits
                : public function_traits<decltype(&T::operator())>
        {};

        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...) const>
        // we specialize for pointers to member function
        {
            enum { arity = sizeof...(Args) };
            // arity is the number of arguments.

            typedef ReturnType result_type;

            template <size_t i>
            struct arg
            {
                typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
                // the i-th argument is equivalent to the i-th tuple element of a tuple
                // composed of those arguments.
            };
        };

        /* It is an additional to the original function_traits to handle non-const function (with mutable keyword lambda). */

        template <typename ClassType, typename ReturnType, typename... Args>
        struct function_traits<ReturnType(ClassType::*)(Args...)>
        // we specialize for pointers to member function
        {
            enum { arity = sizeof...(Args) };
            // arity is the number of arguments.

            typedef ReturnType result_type;

            template <size_t i>
            struct arg
            {
                typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
                // the i-th argument is equivalent to the i-th tuple element of a tuple
                // composed of those arguments.
            };
        };

        template <typename R>
        inline void completeDefer(AsyncFuture::Deferred<R> defer, const QVector<QFuture<R>> &futures) {
            QList<R> res;
            for (int i = 0 ; i < futures.size() ;i++) {
                res << futures[i].result();
            }
            defer.complete(res);
        }

        template <>
        inline void completeDefer<void>(AsyncFuture::Deferred<void> defer, const QVector<QFuture<void>>& futures) {
            Q_UNUSED(futures);
            defer.complete();
        }

        inline QString key(QObject* object, QString extraKey) {
            return QString("%1-%2").arg(QString::number((long) object, 16) ).arg(extraKey);
        }

        extern QMap<QString, QFuture<void>> debounceStore;

        template <typename T>
        class CustomDeferred : public AsyncFuture::Deferred<T> {
        public:
            void setProgressValue(int value) {
                AsyncFuture::Deferred<T>::deferredFuture->setProgressValue(value);
            }

            void setProgressRange(int min, int max) {
                AsyncFuture::Deferred<T>::deferredFuture->setProgressRange(min, max);
            }

            void reportResult(T value, int index) {
                AsyncFuture::Deferred<T>::deferredFuture->reportResult(value, index);
            }

            void finish() {
                AsyncFuture::Deferred<T>::deferredFuture->complete();
            }
        };

        template <>
        class CustomDeferred<void> : public AsyncFuture::Deferred<void> {
        public:
            void setProgressValue(int value) {
                AsyncFuture::Deferred<void>::deferredFuture->setProgressValue(value);
            }

            void setProgressRange(int min, int max) {
                AsyncFuture::Deferred<void>::deferredFuture->setProgressRange(min, max);
            }

            void finish() {
                AsyncFuture::Deferred<void>::deferredFuture->complete();
            }
        };

        template <typename T>
        inline void pipelineReportResult(CustomDeferred<T>& defer, int index, QFuture<T> future) {
            auto value = future.result();
            defer.reportResult(value, index);
        }

        template <>
        inline void pipelineReportResult<void>(CustomDeferred<void>& defer, int index, QFuture<void> future) {
            Q_UNUSED(defer);
            Q_UNUSED(index);
            Q_UNUSED(future);
        }

        template <typename RET, typename ARG>
        class PipelineContext {
        private:
            /// Variables access is not allowed out of the main thread except the initialization

            QPointer<QThreadPool> pool;
            std::function<RET(ARG)> worker;
            int next;
            QList<ARG> input;

            /// no. of running tasks
            int running;

            int completedCount;

            Private::CustomDeferred<RET> defer;

            QList<AsyncFuture::Deferred<RET>> tasks;

            bool closed;

            bool autoDelete;

            void checkDelete() {
                if (autoDelete) {
                    runOnMainThreadVoid([=]() {
                       delete this;
                    });
                }
            }

            void run() {
                if (running >= pool->maxThreadCount() || next >= input.size()) {
                    return;
                }

                int index = next;
                ARG inputValue = input.at(next);
                auto task = tasks.at(next);
                next++;
                running++;

                auto future = QtConcurrent::run(pool, worker, inputValue);
                task.complete(future);
                task.subscribe([=]() {
                    int progressValue = defer.future().progressValue();
                    Private::pipelineReportResult<RET>(defer, index, task.future());

                    defer.setProgressValue(progressValue+1);
                    completedCount++;
                    running--;

                    if (closed && completedCount == tasks.size()) {
                        defer.finish();
                        checkDelete();
                        return;
                    }

                    if (defer.future().isFinished() || defer.future().isCanceled()) {
                        return;
                    }
                    run();
                });
            }

            void _add(AsyncFuture::Deferred<RET> task, ARG value) {
                if (defer.future().isFinished() ||
                    defer.future().isCanceled() ||
                    closed) {
                    task.cancel();
                    return;
                }

                input << value;
                tasks << task;
                defer.setProgressRange(0, tasks.size());
                if (running < pool->maxThreadCount()) {
                    run();
                }
            }

            void _close() {
                closed = true;

                if (running == 0 && (next >= tasks.size() || defer.future().isCanceled())) {
                    defer.finish();
                    checkDelete();
                }
            }

            void init() {
                completedCount = 0;
                next = 0;
                running = 0;
                closed = false;
                autoDelete = false;
                defer.subscribe([]() {}, [=](){
                    closed = true;
                    for (int i = next ; i < tasks.size(); i++) {
                        tasks[i].cancel();
                    }
                    checkDelete();
                });

            }

        public:
            PipelineContext(QThreadPool* pool, std::function<RET(ARG)> worker) : pool(pool), worker(worker){
                init();
            }

            PipelineContext(QThreadPool* pool, std::function<RET(ARG)> worker, QList<ARG> sequence) : pool(pool), worker(worker){
                init();

                input = sequence;
                for (int i = 0 ; i < sequence.size() ; i++) {
                    auto task = AsyncFuture::Deferred<RET>();
                    tasks << task;
                }

                defer.setProgressRange(0, sequence.size());

                for (int i = 0 ; i < pool->maxThreadCount();i++) {
                    run();
                }

            }

            ~PipelineContext() {
            }

            QFuture<RET> add(ARG value) {
                auto res = AsyncFuture::Deferred<RET>();
                runOnMainThreadVoid([=]() {
                    _add(res, value);
                });
                return res.future();
            }

            QFuture<RET> future() {
                return defer.future();
            }

            /// Close the pipeline. No more tasks could be added. The contained future will be terminated automatically once all the tasks finished.
            void close() {
                runOnMainThreadVoid([=]() {
                    _close();
                });
            }

            static QSharedPointer<PipelineContext<RET,ARG>> create(QThreadPool* pool, std::function<RET(ARG)> worker, QList<ARG> input) {

                auto deleter = [](PipelineContext<RET,ARG> *object) {
                    runOnMainThreadVoid([=]() {
                        object->autoDelete = true;
                        object->_close();
                    });
                };

                QSharedPointer<PipelineContext<RET,ARG>> ptr(new PipelineContext<RET,ARG>(pool, worker, input), deleter);
                return ptr;
            }
        };


    } // End of Private namespace

    /// Run a function on main thread. If the current thread is main thread, it will be executed in next tick.
    /// It returns a QFuture to represent the result
    template <typename Functor>
    inline auto runOnMainThread(Functor func) -> QFuture<typename Private::function_traits<Functor>::result_type> {
        typedef typename Private::function_traits<Functor>::result_type RET;
        QObject tmp;
        AsyncFuture::Deferred<RET> defer;
        auto worker = [=]() {
            Private::Value<RET> value;
            value.run(func);
            value.complete(defer);
        };
        QObject::connect(&tmp, &QObject::destroyed, QCoreApplication::instance(), worker, Qt::QueuedConnection);
        return defer.future();
    }

    // Wait for a QFuture to be finished without blocking
    template <typename T>
    inline void await(QFuture<T> future, int timeout = -1) {
        if (future.isFinished()) {
            return;
        }

        QFutureWatcher<T> watcher;
        watcher.setFuture(future);
        QEventLoop loop;

        if (timeout > 0) {
            QTimer::singleShot(timeout, &loop, &QEventLoop::quit);
        }

        QObject::connect(&watcher, SIGNAL(finished()), &loop, SLOT(quit()));

        loop.exec();
    }

    template <typename T, typename Functor>
    void debounce(QObject* context, QString key, QFuture<T> future, Functor functor) {

        QString k = Private::key(context, key);

        auto defer = AsyncFuture::deferred<void>();

        auto cleanup = [=]() {
            if (Private::debounceStore.contains(k) &&
                Private::debounceStore[k] == defer.future()) {
                Private::debounceStore.remove(k);
            }
        };

        defer.subscribe([=]() {
            if (Private::debounceStore.contains(k) &&
                Private::debounceStore[k] == defer.future()) {
                functor();
            }
            cleanup();
        }, cleanup);

        defer.complete(future);

        if (Private::debounceStore.contains(k)) {
            Private::debounceStore[k].cancel();
        }
        Private::debounceStore[k] = defer.future();
    }

    template <typename RET, typename ARG>
    class Queue {
    private:
        class Context {
        public:
            QPointer<QThreadPool> pool;
            std::function<RET(ARG)> worker;
            AsyncFuture::Deferred<RET> defer;
            QQueue<ARG> queue;

            // Is the head started?
            bool started;
        };

    public:
        Queue(QThreadPool* pool, std::function<RET(ARG)> worker) : d(QSharedPointer<Context>::create()) {
            d->pool = pool;
            d->worker = worker;
            d->started = false;
        }

        int count() {
            return d->queue.count();
        }

        // The head's future
        QFuture<RET> future() {
            return d->defer.future();
        }

        void enqueue(ARG arg) {
            d->queue.enqueue(arg);
        }

        ARG head() {
            return d->queue.head();
        }

        void dequeue() {
            d->defer = AsyncFuture::deferred<RET>();
            d->started = false;
            if (d->queue.count() > 0) {
                d->queue.dequeue();
            }
        }

        QFuture<RET> run() {
            // Run the head item
            if (d->started || d->queue.count() == 0) {
                return d->defer.future();
            }
            d->started = true;
            auto f = QtConcurrent::run(d->pool, d->worker, d->queue.head());
            d->defer.complete(f);
            return d->defer.future();
        }

    private:
        QSharedPointer<Context> d;
    };

    template <typename Functor>
    inline auto queue(QThreadPool*pool, Functor func) -> Queue<
        typename Private::function_traits<Functor>::result_type,
        typename Private::function_traits<Functor>::template arg<0>::type
    >{
        typedef typename Private::function_traits<Functor>::template arg<0>::type ARG;
        typedef typename Private::function_traits<Functor>::result_type RET;

        Queue<RET,ARG> queue(pool, func);


        return queue;
    }

    template <typename RET, typename ARG>
    class Pipeline {

        QSharedPointer<Private::PipelineContext<RET, ARG>> d;

    public:
        Pipeline() {
        }

        Pipeline(QThreadPool* pool, std::function<RET(ARG)> worker, QList<ARG> input = QList<ARG>()) : d(Private::PipelineContext<RET, ARG>::create(pool, worker, input)) {
        }

        QFuture<RET> add(ARG value) {
            QFuture<RET> future;
            if (d) {
                future = d->add(value);
            }
            return future;
        }

        QFuture<RET> future() {
            QFuture<RET> future;
            if (d) {
                future = d->future();
            }
            return future;
        }

        void close() {
            if (d) {
                d->close();
            }
        }

    };


    template <typename Functor>
    inline auto pipeline(QThreadPool*pool, Functor func) -> Pipeline<
        typename Private::function_traits<Functor>::result_type,
        typename Private::function_traits<Functor>::template arg<0>::type
    >{
        typedef typename Private::function_traits<Functor>::template arg<0>::type ARG;
        typedef typename Private::function_traits<Functor>::result_type RET;

        Pipeline<RET,ARG> res(pool, func);

        return res;
    }

    template <typename Functor, typename ARG>
    inline auto pipeline(QThreadPool*pool, Functor func, QList<ARG> input) -> Pipeline<
        typename Private::function_traits<Functor>::result_type,
        typename Private::function_traits<Functor>::template arg<0>::type
    >{
        typedef typename Private::function_traits<Functor>::template arg<0>::type A;
        typedef typename Private::function_traits<Functor>::result_type RET;

        Pipeline<RET, A> res(pool, func, input);

        return res;
    }

    template <typename Sequence, typename Functor>
    inline auto mapped(QThreadPool*pool, Sequence input, Functor func) -> QFuture<typename Private::function_traits<Functor>::result_type>{
        auto handler = pipeline(pool, func, input);
        handler.close();

        return handler.future();
    }

    template <typename Sequence, typename Functor>
    inline auto mapped(Sequence input, Functor func) -> QFuture<typename Private::function_traits<Functor>::result_type>{
        return mapped(QThreadPool::globalInstance(), input, func);
    }

    template <typename Sequence, typename Functor>
    inline auto blockingMapped(QThreadPool*pool, Sequence input, Functor func) -> QList<typename Private::function_traits<Functor>::result_type>{
        auto f = mapped(pool, input, func);
        await(f);
        return f.results();
    }

}

