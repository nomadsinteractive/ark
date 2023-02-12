#include "core/base/observer.h"

#include "core/inf/runnable.h"
#include "core/util/holder_util.h"

namespace ark {

Observer::Observer(sp<Runnable> callback, bool oneshot)
    : _callback(std::move(callback)), _oneshot(oneshot)
{
}

void Observer::traverse(const Holder::Visitor& visitor)
{
    HolderUtil::visit(_callback, visitor);
}

void Observer::notify()
{
    sp<Runnable> callback = std::move(_callback);
    if(callback)
        callback->run();
    if(!_oneshot)
        _callback = std::move(callback);
}

void Observer::setCallback(sp<Runnable> callback)
{
    _callback = std::move(callback);
}

}
