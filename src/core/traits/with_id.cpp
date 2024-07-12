#include "core/traits/with_id.h"

#include "core/base/constants.h"

namespace ark {

WithId::WithId()
{
}

TypeId WithId::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithId::onWire(const WiringContext& context)
{
}

IdType WithId::id() const
{
    return _id;
}

}
