#include "graphics/traits/position.h"

#include "graphics/base/v3.h"

namespace ark {

Position::Position(sp<Vec3> xyz)
    : _xyz(std::move(xyz))
{
}

bool Position::update(uint64_t timestamp)
{
    return _xyz->update(timestamp);
}

V3 Position::val()
{
    return _xyz->val();
}

const sp<Vec3>& Position::xyz() const
{
    return _xyz;
}

}
