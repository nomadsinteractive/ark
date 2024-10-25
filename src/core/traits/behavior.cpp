#include "core/traits/behavior.h"

#include "core/ark.h"

#include "app/base/application_context.h"

namespace ark {

namespace {

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
}

sp<Runnable> Behavior::getMethod(const String& name)
{
    return nullptr;
}

}
