#ifndef ARK_CORE_IMPL_RUNNABLE_RUNNABLE_BY_FUNCTION_H_
#define ARK_CORE_IMPL_RUNNABLE_RUNNABLE_BY_FUNCTION_H_

#include <functional>

#include "core/inf/runnable.h"

namespace ark {

class RunnableByFunction : public Runnable {
public:
    RunnableByFunction(std::function<void()> function);

    virtual void run() override;

private:
    std::function<void()> _function;
};

}

#endif
