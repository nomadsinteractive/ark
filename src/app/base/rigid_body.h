#ifndef ARK_APP_BASE_RIGID_BODY_H_
#define ARK_APP_BASE_RIGID_BODY_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"

namespace ark {

class ARK_API RigidBody {
public:
    virtual ~RigidBody() = default;

    RigidBody(uint32_t id, Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Numeric>& rotation);

//  [[script::bindings::auto]]
    virtual void dispose() = 0;

//  [[script::bindings::property]]
    uint32_t id() const;
//  [[script::bindings::property]]
    Collider::BodyType type() const;

//  [[script::bindings::property]]
    V2 xy() const;
//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;

//  [[script::bindings::property]]
    const sp<VV>& position() const;
//  [[script::bindings::property]]
    const sp<Size>& size() const;
//  [[script::bindings::property]]
    const sp<Numeric>& rotation() const;

//  [[script::bindings::property]]
    virtual const sp<CollisionCallback>& collisionCallback() const = 0;
//  [[script::bindings::property]]
    virtual void setCollisionCallback(const sp<CollisionCallback>& collisionCallback) = 0;

protected:
    uint32_t _id;
    Collider::BodyType _type;
    sp<VV> _position;
    sp<Size> _size;
    sp<Numeric> _rotation;
};

}

#endif
