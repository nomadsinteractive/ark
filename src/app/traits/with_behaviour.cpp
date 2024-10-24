#include "app/traits/with_behaviour.h"

#include "core/ark.h"
#include "core/base/constants.h"
#include "core/util/holder_util.h"

namespace ark {

WithBehaviour::WithBehaviour(Box delegate)
    : _delegate(std::move(delegate))
{
}

TypeId WithBehaviour::onPoll(WiringContext& /*context*/)
{
    return constants::TYPE_ID_NONE;
}

void WithBehaviour::onWire(const WiringContext& /*context*/)
{
}

void WithBehaviour::traverse(const Visitor& visitor)
{
    visitor(_delegate);
}

}
