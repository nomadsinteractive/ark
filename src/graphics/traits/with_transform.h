#pragma once

#include "core/base/api.h"
#include "core/inf/wirable.h"

namespace ark {

class ARK_API WithTransform final : public Wirable {
public:
//  [[script::bindings::auto]]
    WithTransform(sp<Mat4> transform);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

//  [[script::bindings::property]]
    const sp<Mat4>& transform() const;

private:
    sp<Mat4> _transform;
};

}
