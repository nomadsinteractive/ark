#include "graphics/traits/position.h"

namespace ark {

Position::Position(sp<Vec3> xyz)
    : _xyz(std::move(xyz))
{
}

const sp<Vec3>& Position::xyz() const
{
    return _xyz;
}

}
