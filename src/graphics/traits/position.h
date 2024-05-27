#pragma once

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API Position : public Vec3 {
public:
//  [[script::bindings::auto]]
    Position(sp<Vec3> xyz);

    bool update(uint64_t timestamp) override;
    V3 val() override;

//  [[script::bindings::auto]]
    const sp<Vec3>& xyz() const;

private:
    sp<Vec3> _xyz;
};

} // ark
