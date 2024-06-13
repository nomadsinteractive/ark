#include "app/traits/behaviour.h"

#include "core/ark.h"
#include "core/base/constants.h"
#include "core/util/holder_util.h"

namespace ark {

Behaviour::Behaviour(Box delegate)
    : _delegate(std::move(delegate))
{
}

TypeId Behaviour::onPoll(WiringContext& /*context*/)
{
    return constants::TYPE_ID_NONE;
}

void Behaviour::onWire(const WiringContext& /*context*/)
{
}

void Behaviour::traverse(const Visitor& visitor)
{
    visitor(_delegate);
}

}
