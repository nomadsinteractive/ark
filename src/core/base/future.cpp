#include "core/base/future.h"

#include "core/base/observer.h"
#include "core/util/boolean_type.h"

namespace ark {

Future::Future(sp<Boolean> canceled, sp<Runnable> observer)
    : _done(nullptr, false), _canceled(std::move(canceled), false), _observer(std::move(observer)) {
}

void Future::cancel()
{
    _canceled.reset(true);
}

void Future::done()
{
    _done.reset(true);
    if(const sp<Runnable> observer = std::move(_observer))
        observer->run();
}

sp<Boolean> Future::isCanceled() const
{
    return _canceled.toVar();
}

sp<Boolean> Future::isDone() const
{
    return _done.toVar();
}

sp<Boolean> Future::isDoneOrCanceled() const
{
    return BooleanType::__or__(_done.toVar(), _canceled.toVar());
}

}
