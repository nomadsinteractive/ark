#include "graphics/traits/position.h"

#include "core/impl/variable/variable_dirty.h"
#include "graphics/base/v3.h"

namespace ark {

Position::Position(sp<Vec3> position)
    : Wrapper(std::move(position))
{
}

void Position::reset(sp<Vec3> position)
{
    VariableDirty<V3>::reset(*this, std::move(position));
}

bool Position::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

V3 Position::val()
{
    return _wrapped->val();
}

TypeId Position::onPoll(WiringContext& context)
{
    return Type<Position>::id();
}

void Position::onWire(const WiringContext& context)
{
}

}
