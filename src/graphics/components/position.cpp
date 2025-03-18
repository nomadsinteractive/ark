#include "graphics/components/position.h"

#include "core/impl/variable/variable_dirty_mark.h"
#include "graphics/base/v3.h"

namespace ark {

Position::Position(sp<Vec3> position)
    : Wrapper(std::move(position))
{
}

void Position::reset(sp<Vec3> position)
{
    VariableDirtyMark<V3>::markDirty(*this, std::move(position));
}

bool Position::update(uint64_t timestamp)
{
    return _wrapped->update(timestamp);
}

V3 Position::val()
{
    return _wrapped->val();
}

}
