#pragma once

#include "core/base/api.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::extends(Vec3)]]
class ARK_API Translation final : public Vec3 {
public:
//  [[script::bindings::constructor]]
    Translation(sp<Vec3> translate);
    Translation(sp<Vec3> delegate, sp<Wrapper<Vec3>> wrapper);

    bool update(uint64_t timestamp) override;
    V3 val() override;

//  [[script::bindings::auto]]
    void reset(sp<Vec3> position);

//  [[script::bindings::auto]]
    sp<Mat4> toMatrix() const;

private:
    sp<Vec3> _delegate;
    sp<Wrapper<Vec3>> _wrapper;
};

}
