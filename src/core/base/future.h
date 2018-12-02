#ifndef ARK_CORE_BASE_FUTURE_H_
#define ARK_CORE_BASE_FUTURE_H_

#include "core/base/api.h"

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

private:
    bool _cancelled;
    bool _done;
};

}

#endif
