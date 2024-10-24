#pragma once

#include <vector>

#include "core/inf/debris.h"
#include "core/inf/wirable.h"
#include "core/types/weak_ptr.h"

namespace ark {

class WithDebris final : Wirable, Debris {
public:

    void track(sp<Debris> debris);

    static sp<WithDebris> ensureComponent(const WiringContext& context);

private:

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void traverse(const Visitor& visitor) override;

private:
    std::vector<WeakPtr<Debris>> _debris;
};

}
