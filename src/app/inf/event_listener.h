#ifndef ARK_APP_INF_EVENT_LISTENER_H_
#define ARK_APP_INF_EVENT_LISTENER_H_

#include "core/base/api.h"
#include "app/forwarding.h"

namespace ark {

class ARK_API EventListener {
public:
    virtual ~EventListener() = default;

    virtual bool onEvent(const Event& event) = 0;
};

}

#endif
