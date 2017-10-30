#ifndef ARK_CORE_INF_RUNNABLE_H_
#define ARK_CORE_INF_RUNNABLE_H_

#include "core/base/api.h"

namespace ark {

class ARK_API Runnable {
public:
    virtual ~Runnable() = default;

//  [[script::bindings::auto]]
    virtual void run() = 0;
};

}

#endif
