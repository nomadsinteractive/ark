#include "graphics/base/bounds.h"

#include "core/util/math.h"

#include "graphics/base/size.h"

namespace ark {

Bounds::Bounds(const sp<Vec>& position, const sp<Size>& size)
    : _center(position), _size(size)
{
}

bool Bounds::ptin(const V& pt) const
{
    const V c = _center->val();
    const V s = _size->val();
    for(int32_t i = 0; i < DIMENSIONS; i++)
        if(!Math::between(c[i] + s[i] / 2.0f, c[i] - s[i] / 2.0f, pt[i]))
            return false;
    return true;
}

const sp<Vec>& Bounds::center() const
{
    return _center;
}

const SafePtr<Size>& Bounds::size()
{
    return _size;
}

}

