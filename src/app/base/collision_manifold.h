#ifndef ARK_APP_BASE_COLLISION_MANIFOLD_H_
#define ARK_APP_BASE_COLLISION_MANIFOLD_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API CollisionManifold {
public:
    CollisionManifold(const V& normal);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(CollisionManifold);

//  [[script::bindings::property]]
    const V& normal() const;

private:
    V _normal;
};

}

#endif
