#include "core/base/future.h"

#include "core/base/observer.h"
#include "core/util/boolean_type.h"

namespace ark {

Future::Future(sp<Boolean> canceled, sp<Runnable> observer, const uint32_t countDown)
    : _done(nullptr, false), _canceled(std::move(canceled), false), _observer(std::move(observer)), _count_down(countDown)
{
}

void Future::cancel()
{
    _canceled.reset(true);
}

void Future::notify()
{
    if(_canceled.val())
        return;

    if(_count_down == 1)
    {
        _done.reset(true);
        if(const sp<Runnable> observer = std::move(_observer))
            observer->run();
    }
    else
    {
        if(_count_down > 0)
            -- _count_down;
        if(sp<Runnable> observer = std::move(_observer))
        {
            observer->run();
            _observer = std::move(observer);
        }
    }
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
