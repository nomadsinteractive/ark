#include "core/impl/runnable/runnable_with_callback.h"

namespace ark {

RunnableWithCallback::RunnableWithCallback(const sp<Runnable>& delegate, const Observer& callback)
    : _delegate(delegate), _callback(callback)
{
}

void RunnableWithCallback::run()
{
    _delegate->run();
    _callback.update();
}

}
