#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::extends(Boolean)]]
class ARK_API Visibility final : public Boolean {
public:
//  [[script::bindings::auto]]
    Visibility(bool visible = true);
//  [[script::bindings::auto]]
    Visibility(const sp<Boolean>& visible);

    bool val() override;
    bool update(uint32_t tick) override;

//  [[script::bindings::auto]]
    void show();
//  [[script::bindings::auto]]
    void hide();
//  [[script::bindings::auto]]
    void reset(bool visible);
//  [[script::bindings::auto]]
    void reset(sp<Boolean> visible);

private:
    sp<BooleanWrapper> _visible;
};

}
