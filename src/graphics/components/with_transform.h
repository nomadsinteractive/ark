#pragma once

#include "core/base/api.h"
#include "core/inf/wirable.h"

namespace ark {

class ARK_API WithTransform final : public Wirable {
public:
    WithTransform(sp<Transform> transform);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context, const Box& self) override;

private:
    sp<Transform> _transform;
};

}
