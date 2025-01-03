#include "core/util/wirable_type.h"

#include "core/inf/wirable.h"

namespace ark {

void WirableType::wireAll(Traits& traits)
{
    Wirable::WiringContext context(traits);

    for(const auto& [k, v] : traits.traits())
        if(const sp<Wirable> wirable = v.as<Wirable>())
            if(const TypeId typeId = wirable->onPoll(context); typeId != constants::TYPE_ID_NONE)
                traits.add(typeId, v);
    for(const auto& [k, v] : traits.traits())
        if(const sp<Wirable> wirable = v.as<Wirable>())
            wirable->onWire(context);
}

}
