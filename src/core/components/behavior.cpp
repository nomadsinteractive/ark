#include "core/components/behavior.h"

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

void Behavior::onWire(const WiringContext& context, const Box& self)
{
    _with_debris.onWire(context, self);
}

void Behavior::traverse(const Visitor& visitor)
{
    visitor(_delegate);
    _with_debris.traverse(visitor);
}

sp<Runnable> Behavior::getRunnable(StringView name)
{
    Box function = _interpreter->attr(_delegate, name);
    CHECK(function, "Object has no attribute \"%s\"", name.data());
    sp<RunnableImpl> runnable = sp<RunnableImpl>::make(_interpreter, function);
    _with_debris.track(runnable);
    return runnable;
}

Behavior::Method::Method(sp<Interpreter> interpreter, Box function)
    : _interpreter(std::move(interpreter)), _function(std::move(function))
{
}

void Behavior::Method::call(const Interpreter::Arguments& args) const
{
    _interpreter->call(_function, args);
}

void Behavior::Method::traverse(const Visitor& visitor)
{
    visitor(_function);
}

sp<Behavior::Method> Behavior::getMethod(StringView name)
{
    Box function = _interpreter->attr(_delegate, name);
    CHECK(function, "Object has no attribute \"%s\"", name.data());
    sp<Method> method = sp<Method>::make(_interpreter, function);
    _with_debris.track(method);
    return method;
}

}
