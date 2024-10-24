#pragma once

#include "core/inf/debris.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"

namespace ark {

class WithBehaviour final : public Wirable, public Debris {
public:
    WithBehaviour(Box delegate);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void traverse(const Visitor& visitor) override;

private:
    Box _delegate;
};

}
