#ifndef ARK_CORE_INF_MESSAGE_LOOP_H_
#define ARK_CORE_INF_MESSAGE_LOOP_H_

#include <functional>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API MessageLoop {
public:
    virtual ~MessageLoop() = default;

    virtual void post(const sp<Runnable>& task, float delay) = 0;
    virtual void schedule(const sp<Runnable>& task, float interval) = 0;
};

}

#endif
