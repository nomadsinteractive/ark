#ifndef ARK_CORE_IMPL_RUNNABLE_RUNNABLE_COMPOSITE_H_
#define ARK_CORE_IMPL_RUNNABLE_RUNNABLE_COMPOSITE_H_

#include <vector>

#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class RunnableComposite : public Runnable {
public:
    RunnableComposite(std::vector<sp<Runnable>> composites);

    virtual void run() override;

private:
    std::vector<sp<Runnable>> _composites;
};

}

#endif
