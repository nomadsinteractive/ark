#ifndef ARK_APP_INF_COLLIDER_H_
#define ARK_APP_INF_COLLIDER_H_

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Collider {
public:
//  [[script::bindings::enumeration]]
    enum BodyShape {
        BODY_SHAPE_BOX,
        BODY_SHAPE_CIRCLE
    };

//  [[script::bindings::enumeration]]
    enum BodyType {
        BODY_TYPE_DYNAMIC,
        BODY_TYPE_KINEMATIC,
        BODY_TYPE_STATIC
    };

public:
    virtual ~Collider() = default;

//  [[script::bindings::auto]]
    virtual sp<RigidBody> createBody(Collider::BodyType type, int32_t shape, const sp<VV>& position, const sp<Size>& size) = 0;

};

}

#endif
