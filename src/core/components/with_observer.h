#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API WithObserver {
public:
    WithObserver(sp<Observer> observer);

    const sp<Observer>& observer();

    void notify() const;

private:
    sp<Observer> _observer;
};

}
