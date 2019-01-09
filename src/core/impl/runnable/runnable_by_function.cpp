#include "core/impl/runnable/runnable_by_function.h"

namespace ark {

RunnableByFunction::RunnableByFunction(std::function<void()> function)
    : _function(std::move(function))
{
}

void RunnableByFunction::run()
{
    _function();
}

}
