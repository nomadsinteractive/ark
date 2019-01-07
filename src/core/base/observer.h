#ifndef ARK_CORE_BASE_OBSERVER_H_
#define ARK_CORE_BASE_OBSERVER_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Observer : public Boolean {
public:
    Observer(bool dirty = true, const sp<Runnable>& handler = nullptr, bool oneshot = false);

    virtual bool val() override;

    bool dirty();

    void update();

private:
    sp<Runnable> _handler;
    bool _oneshot;
    bool _dirty;

    friend class Notifier;
};

}

#endif
