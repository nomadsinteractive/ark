#ifndef ARK_CORE_IMPL_RUNNABLE_RUNNABLE_WITH_CALLBACK_H_
#define ARK_CORE_IMPL_RUNNABLE_RUNNABLE_WITH_CALLBACK_H_

#include "core/base/observer.h"
#include "core/inf/runnable.h"

namespace ark {

class RunnableWithCallback : public Runnable {
public:
    RunnableWithCallback(const sp<Runnable>& delegate, const Observer& callback);

    virtual void run() override;

private:
    sp<Runnable> _delegate;
    Observer _callback;

};


}


#endif
