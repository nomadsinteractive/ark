#include "core/impl/runnable/runnable_by_function_with_expired.h"

#include "core/impl/variable/variable_wrapper.h"

namespace ark {

RunnableByFunctionWithExpired::RunnableByFunctionWithExpired(const std::function<bool()>& function)
    : Disposable(false), _function(function)
{
}

void RunnableByFunctionWithExpired::run()
{
    if(!_disposed->val())
        _disposed->set(!_function());
}

}
