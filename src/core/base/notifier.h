#ifndef ARK_CORE_BASE_NOTIFIER_H_
#define ARK_CORE_BASE_NOTIFIER_H_

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Notifier {
public:
    Notifier(const sp<Runnable>& notifier);

    void notify();

private:
    sp<Runnable> _runnable;
    bool _notifying;
};

}

#endif
