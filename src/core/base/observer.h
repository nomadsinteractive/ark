#ifndef ARK_CORE_BASE_OBSERVER_H_
#define ARK_CORE_BASE_OBSERVER_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Observer {
public:
    Observer(const sp<Runnable>& handler);

    void update();
    void updateOnce();

private:
    sp<Runnable> _handler;
    bool _notifying;
};

}

#endif
