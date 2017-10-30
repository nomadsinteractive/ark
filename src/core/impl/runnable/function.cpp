#include "core/impl/runnable/function.h"

namespace ark {

Function::Function(const sp<Runnable>& delegate)
    : _delegate(delegate), _enabled(true)
{
}

void Function::run()
{
    if(_enabled && _delegate)
        _delegate->run();
}

void Function::setRunnableDelegate(const sp<Runnable>& delegate)
{
    _delegate = delegate;
}

void Function::reset()
{
    _delegate = nullptr;
}

void Function::enable()
{
    _enabled = true;
}

void Function::disable()
{
    _enabled = false;
}

}
