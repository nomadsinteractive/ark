#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/debris.h"
#include "core/inf/interpreter.h"
#include "core/inf/wirable.h"
#include "core/components/with_debris.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[script::bindings::debris]]
class ARK_API Behavior final : public Wirable, public Debris {
public:
//  [[script::bindings::auto]]
    Behavior(Box delegate);

    void onWire(const WiringContext& context) override;
    void traverse(const Visitor& visitor) override;

//  [[script::bindings::getprop]]
    sp<Runnable> getRunnable(StringView name);

    class Method final : public Debris {
    public:
        Method(sp<Interpreter> interpreter, Box function);

        void call(const Interpreter::Arguments& args) const;

        void traverse(const Visitor& visitor) override;

    private:
        sp<Interpreter> _interpreter;
        Box _function;
    };

    sp<Method> getMethod(StringView name);

private:
    sp<Interpreter> _interpreter;
    Box _delegate;

    WithDebris _with_debris;
};

}