#include "graphics/traits/bounds.h"

#include "core/util/math.h"
#include "core/util/updatable_util.h"

#include "graphics/base/size.h"

namespace ark {

Bounds::Bounds(sp<Vec3> position, sp<Size> size)
    : _center(std::move(position)), _size(std::move(size))
{
}

bool Bounds::ptin(const V3& pt) const
{
    const V3 c = _center->val();
    const V3 s = _size->val();
    for(int32_t i = 0; i < 3; i++)
        if(!Math::between(c[i] + s[i] / 2.0f, c[i] - s[i] / 2.0f, pt[i]))
            return false;
    return true;
}

const sp<Vec3>& Bounds::center() const
{
    return _center;
}

void Bounds::setCenter(sp<Vec3> center)
{
    _center = std::move(center);
}

const sp<Size>& Bounds::size()
{
    return _size;
}

void Bounds::setSize(sp<Size> size)
{
    _size = std::move(size);
}

bool Bounds::update(uint64_t timestamp)
{
    return UpdatableUtil::update(timestamp, _center, _size);
}

}
