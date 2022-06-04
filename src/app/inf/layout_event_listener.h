#ifndef ARK_APP_INF_LAYOUT_EVENT_LISTENER_H_
#define ARK_APP_INF_LAYOUT_EVENT_LISTENER_H_

#include "core/base/api.h"


#include "app/forwarding.h"

namespace ark {

class ARK_API LayoutEventListener {
public:
    virtual ~LayoutEventListener() = default;

    virtual bool onEvent(const Event& event, float x, float y, bool ptin) = 0;
};

}

#endif
