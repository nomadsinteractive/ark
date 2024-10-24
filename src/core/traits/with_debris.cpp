#include "core/traits/with_debris.h"

namespace ark {

void WithDebris::track(sp<Debris> debris)
{
    _debris.emplace_back(std::move(debris));
}

sp<WithDebris> WithDebris::ensureComponent(const WiringContext& context)
{
    if(sp<WithDebris> component = context.getComponent<WithDebris>())
        return component;

    sp<WithDebris> component = sp<WithDebris>::make();
    const_cast<WiringContext&>(context).addComponent(component);
    return component;
}

TypeId WithDebris::onPoll(WiringContext& context)
{
    return constants::TYPE_ID_NONE;
}

void WithDebris::onWire(const WiringContext& context)
{
}

void WithDebris::traverse(const Visitor& visitor)
{
    for(auto iter = _debris.begin(); iter != _debris.end(); )
        if(const sp<Debris> debris = iter->lock())
        {
            debris->traverse(visitor);
            ++iter;
        }
        else
            iter = _debris.erase(iter);
}

}
