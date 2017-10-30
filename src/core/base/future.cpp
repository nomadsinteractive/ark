#include "core/base/future.h"

namespace ark {

Future::Future()
    : _cancelled(false), _done(false) {
}

void Future::cancel()
{
    _cancelled = true;
}

void Future::done()
{
    _done = true;
}

bool Future::isCancelled() const
{
    return _cancelled;
}

bool Future::isDone() const
{
    return _done;
}

}
