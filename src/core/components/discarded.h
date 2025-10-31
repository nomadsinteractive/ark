#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Discarded final : public Boolean {
public:
//  [[script::bindings::auto]]
    Discarded(sp<Boolean> discarded);
//  [[script::bindings::auto]]
    Discarded(bool discarded = false);

    bool val() override;
    bool update(uint32_t tick) override;

//  [[script::bindings::auto]]
    void discard() const;

//  [[script::bindings::auto]]
    void set(sp<Boolean> discarded) const;
//  [[script::bindings::auto]]
    void set(bool discarded) const;

private:
    sp<BooleanWrapper> _discarded;
};

}
