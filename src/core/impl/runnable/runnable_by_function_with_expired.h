#ifndef ARK_CORE_IMPL_RUNNABLE_RUNNABLE_BY_FUNCTION_WITH_EXPIRABLE_H_
#define ARK_CORE_IMPL_RUNNABLE_RUNNABLE_BY_FUNCTION_WITH_EXPIRABLE_H_

#include <functional>

#include "core/inf/runnable.h"
#include "core/epi/expired.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[core::class]]
class RunnableByFunctionWithExpired : public Runnable, public Expired {
public:
    RunnableByFunctionWithExpired(const std::function<bool()>& function);

    virtual void run() override;

private:
    std::function<bool()> _function;
};

}

#endif
