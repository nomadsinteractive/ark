#ifndef ARK_CORE_BASE_NOTIFIER_H_
#define ARK_CORE_BASE_NOTIFIER_H_

#include "core/collection/filtered_list.h"

namespace ark {

class Notifier {
public:

    void notify();

    sp<Observer> createObserver(const sp<Runnable>& handler = nullptr, bool oneshot = false);

private:
    WeakRefList<Observer> _observers;
};


}


#endif
