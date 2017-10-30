#ifndef ARK_CORE_IMPL_FUNCTION_H_
#define ARK_CORE_IMPL_FUNCTION_H_

#include "core/base/api.h"
#include "core/inf/runnable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[core::class]]
class ARK_API Function : public Runnable {
public:
//[[script::bindings::auto]]
    Function(const sp<Runnable>& delegate);

//[[script::bindings::auto]]
    virtual void run() override;

//[[script::bindings::auto]]
    void setRunnableDelegate(const sp<Runnable>& delegate);
//[[script::bindings::auto]]
    void reset();
//[[script::bindings::auto]]
    void enable();
//[[script::bindings::auto]]
    void disable();

private:
    sp<Runnable> _delegate;
    bool _enabled;
};

}

#endif
