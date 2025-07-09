#include "core/components/with_callback.h"

#include "core/base/observer.h"

namespace ark {

WithCallback::WithCallback(sp<Runnable> callback)
    : _callback(std::move(callback))
{
}

void WithCallback::notify() const
{
    if(_callback)
        _callback->run();
}

}
