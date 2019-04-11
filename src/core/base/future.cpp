#include "core/base/future.h"

namespace ark {

Future::Future()
    : _cancelled(sp<Boolean::Impl>::make(false)), _done(false) {
}

void Future::cancel()
{
    _cancelled->set(true);
}

void Future::done()
{
    _done = true;
}

bool Future::isCancelled() const
{
    return _cancelled->val();
}

bool Future::isDone() const
{
    return _done;
}

sp<Boolean> Future::cancelled() const
{
    return _cancelled;
}

}
