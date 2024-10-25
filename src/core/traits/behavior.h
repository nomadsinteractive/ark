#pragma once

#include "core/forwarding.h"
#include "core/inf/debris.h"
#include "core/inf/wirable.h"
#include "core/traits/with_debris.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Behavior final : public Wirable, public Debris {
public:
//  [[script::bindings::auto]]
    Behavior(Box delegate);

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    void traverse(const Visitor& visitor) override;

//  [[script::bindings::getprop]]
    sp<Runnable> getMethod(const String& name);

private:
    template<typename T> void track(const sp<T>& object) {
        if(sp<Debris> debris = object.tryCast<Debris>())
            _with_debris.track(std::move(debris));
    }

private:
    sp<Interpreter> _interpreter;
    Box _delegate;

    WithDebris _with_debris;
};

}
