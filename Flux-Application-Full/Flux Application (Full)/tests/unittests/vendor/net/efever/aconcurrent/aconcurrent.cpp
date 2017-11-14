#include <aconcurrent.h>

using namespace AConcurrent;

QMap<QString, QFuture<void>> AConcurrent::Private::debounceStore;
