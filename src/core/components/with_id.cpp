#include "core/components/with_id.h"

#include "core/base/constants.h"

namespace ark {

WithId::WithId()
{
}

void WithId::onWire(const WiringContext& context, const Box& self)
{
}

RefId WithId::id() const
{
    return _id;
}

}
