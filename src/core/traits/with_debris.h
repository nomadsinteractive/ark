#pragma once

#include "core/inf/debris.h"
#include "core/inf/wirable.h"

namespace ark {

class WithDebris final : public Wirable, public Debris {
public:
    WithDebris();
    DISALLOW_COPY_AND_ASSIGN(WithDebris);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void traverse(const Visitor& visitor) override;

    void track(const sp<Debris>& debris) const;

private:
    struct Tracker;
    static sp<Tracker> ensureTracker(const WiringContext& context);

    sp<Tracker> _tracker;
};

}
