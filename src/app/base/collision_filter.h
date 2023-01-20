#ifndef ARK_APP_BASE_COLLISION_FILTER_H_
#define ARK_APP_BASE_COLLISION_FILTER_H_

#include "core/base/api.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API CollisionFilter {
public:
//  [[script::bindings::auto]]
    CollisionFilter(uint32_t categoryBits = 1, uint32_t maskBits = std::numeric_limits<uint32_t>::max(), int32_t groupIndex = 0);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(CollisionFilter);

//  [[script::bindings::property]]
    uint32_t categoryBits() const;
//  [[script::bindings::property]]
    void setCategoryBits(uint32_t categoryBits);

//  [[script::bindings::property]]
    uint32_t maskBits() const;
//  [[script::bindings::property]]
    void setMaskBits(uint32_t maskBits);

//  [[script::bindings::property]]
    int32_t groupIndex() const;
//  [[script::bindings::property]]
    void setGroupIndex(int32_t groupIndex);

    bool collisionTest(const CollisionFilter& other) const;

private:
    uint32_t _category_bits;
    uint32_t _mask_bits;

    int32_t _group_index;
};

}

#endif
