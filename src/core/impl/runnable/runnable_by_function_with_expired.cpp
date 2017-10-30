#include "core/impl/runnable/runnable_by_function_with_expired.h"

namespace ark {

RunnableByFunctionWithExpired::RunnableByFunctionWithExpired(const std::function<bool()>& function)
    : Expired(false), _function(function)
{
}

void RunnableByFunctionWithExpired::run()
{
    if(!_expired)
        _expired = !_function();
}

}
