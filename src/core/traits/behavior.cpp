#include "core/traits/behavior.h"

#include "core/ark.h"

#include "app/base/application_context.h"
#include "core/inf/interpreter.h"

namespace ark {

namespace {

class RunnableImpl final : public Runnable, public Debris {
public:
    RunnableImpl(sp<Interpreter> interpreter, Box function)
        : _interpreter(std::move(interpreter)), _function(std::move(function))
    {
    }

    void run() override
    {
        _interpreter->call(_function, {});
    }

    void traverse(const Visitor& visitor) override
    {
        visitor(_function);
    }

private:
    sp<Interpreter> _interpreter;
    Box _function;
};

}

Behavior::Behavior(Box delegate)
    : _interpreter(Ark::instance().applicationContext()->interpreter()), _delegate(std::move(delegate))
{
}

TypeId Behavior::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void Behavior::onWire(const WiringContext& context)
{
}

void Behavior::traverse(const Visitor& visitor)
{
    _with_debris.traverse(visitor);
}

sp<Runnable> Behavior::getMethod(const String& name)
{
    Box function = _interpreter->attr(_delegate, name);
    CHECK(function, "Object has no attribute \"%s\"", name.c_str());
    sp<RunnableImpl> runnable = sp<RunnableImpl>::make(_interpreter, function);
    _with_debris.track(runnable);
    return runnable;
}

}
