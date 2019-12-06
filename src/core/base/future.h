#ifndef ARK_CORE_BASE_FUTURE_H_
#define ARK_CORE_BASE_FUTURE_H_

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Future {
public:
//  [[script::bindings::auto]]
    Future();

//  [[script::bindings::auto]]
    void cancel();
//  [[script::bindings::auto]]
    void done();

//  [[script::bindings::property]]
    bool isCancelled() const;
//  [[script::bindings::property]]
    bool isDone() const;

    sp<Boolean> cancelled() const;

private:
    sp<Boolean::Impl> _cancelled;
    bool _done;
};

}

#endif
