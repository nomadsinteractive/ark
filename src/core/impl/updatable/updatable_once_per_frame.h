#pragma once

#include "core/inf/updatable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class UpdatableOncePerFrame : public Updatable {
public:
    UpdatableOncePerFrame(sp<Updatable> delegate);

    virtual bool update(uint64_t timestamp) override;

private:
    sp<Updatable> _delegate;

    bool _last_update_value;
    uint64_t _last_update_timestamp;
};

}
