#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/inf/wirable.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::extends(Vec3)]]
class ARK_API Position final : public Wrapper<Vec3>, public Vec3 {
public:
//  [[script::bindings::auto]]
    Position(sp<Vec3> position);

//  [[script::bindings::auto]]
    void reset(sp<Vec3> position);

    bool update(uint64_t timestamp) override;
    V3 val() override;

};

}
