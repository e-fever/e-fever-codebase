Enhanced QtConcurrent based on AsyncFuture
-------------------------------------

AConcurrent contains functionality to support concurrent execution of program code. It is an enhanced version of QtConcurrent.


Installation
============

    qpm install net.efever.aconcurrent

API
===

```C++
    #include <aconcurrent.h>
```

**QFuture<R> AConcurrent::runOnMainThread(Functor functor)**

Run a function on the main thread and returns a QFuture<RET> to represent the result of the function. If the current thread is the main thread, it will be executed in next tick.

Example

```C++

auto worker = [=]() {

  // processing

  QFuture<int> future = AConcurrent::runOnMainThread([=]() {
    // Save the result on main thread
    return 0; // no error
  });
  AConcurrent::await(future); // future.waitForFinished();
};

QtConcurrent::run(worker);

```

**QFuture<R> AConcurrent::mapped(Sequence sequence, Functor worker)**

Calls function once for each item in sequence and return a future with each mapped item as a result.
It is similar to the QtConcurrent::mapped() but this version supports lambda function.
The returned QFuture is cancelable.

**QFuture<R> AConcurrent::blockingMapped(Sequence sequence, Functor worker)**

**AConcurrent::await(future)**

Wait until the input future is finished while keeping the event loop running.

**Pipeline AConcurrent::pipeline(QThreadPool* pool, );
