#pragma once

#include "core/inf/holder.h"
#include "core/inf/wirable.h"
#include "core/types/box.h"

namespace ark {

class Behaviour final : public Wirable, public Holder {
public:
    Behaviour(Box delegate);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void traverse(const Visitor& visitor) override;

private:
    Box _delegate;
};

}
