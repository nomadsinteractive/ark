#include "graphics/base/bounds.h"

#include "core/util/math.h"
#include "core/util/variable_util.h"

#include "graphics/base/size.h"

namespace ark {

Bounds::Bounds(const sp<Vec3>& position, const sp<Size>& size)
    : _center(position), _size(size)
{
}

bool Bounds::ptin(const V3& pt) const
{
    const V3 c = _center->val();
    const V3 s = _size->val();
    for(int32_t i = 0; i < DIMENSIONS; i++)
        if(!Math::between(c[i] + s[i] / 2.0f, c[i] - s[i] / 2.0f, pt[i]))
            return false;
    return true;
}

const sp<Vec3>& Bounds::center() const
{
    return _center;
}

const sp<Size>& Bounds::size()
{
    return _size;
}

bool Bounds::update(uint64_t timestamp)
{
    return VariableUtil::update(timestamp, _center, _size);
}

}

