#include "app/base/collision_filter.h"

namespace ark {

CollisionFilter::CollisionFilter(const uint32_t categoryBits, const uint32_t maskBits, const int32_t groupIndex)
    : _category_bits(categoryBits ? categoryBits : std::numeric_limits<uint32_t>::max()), _mask_bits(maskBits), _group_index(groupIndex)
{
}

uint32_t CollisionFilter::categoryBits() const
{
    return _category_bits;
}

void CollisionFilter::setCategoryBits(const uint32_t categoryBits)
{
    _category_bits = categoryBits ? categoryBits : std::numeric_limits<uint32_t>::max();
}

uint32_t CollisionFilter::maskBits() const
{
    return _mask_bits;
}

void CollisionFilter::setMaskBits(const uint32_t maskBits)
{
    _mask_bits = maskBits;
}

int32_t CollisionFilter::groupIndex() const
{
    return _group_index;
}

void CollisionFilter::setGroupIndex(const int32_t groupIndex)
{
    _group_index = groupIndex;
}

bool CollisionFilter::collisionTest(const CollisionFilter& other) const
{
    if(_group_index != 0 && _group_index == other._group_index)
        return _group_index > 0;
    return (_mask_bits & other._category_bits) && (_category_bits & other._mask_bits);
}

}
