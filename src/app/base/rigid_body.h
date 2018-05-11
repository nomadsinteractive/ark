#ifndef ARK_APP_BASE_RIGID_BODY_H_
#define ARK_APP_BASE_RIGID_BODY_H_

#include "core/base/api.h"
#include "core/types/box.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"

namespace ark {

//[[script::bindings::container]]
class ARK_API RigidBody {
public:
    struct Stub {
        Stub(uint32_t id, Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Rotate>& rotate);

        void beginContact(const sp<RigidBody>& rigidBody);
        void endContact(const sp<RigidBody>& rigidBody);

        void beginContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody);
        void endContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody);

        uint32_t _id;
        Collider::BodyType _type;
        sp<VV> _position;
        sp<Size> _size;
        sp<Rotate> _rotate;

        sp<CollisionCallback> _collision_callback;

        Box _tag;
    };

public:
    virtual ~RigidBody() = default;

    RigidBody(uint32_t id, Collider::BodyType type, const sp<VV>& position, const sp<Size>& size, const sp<Rotate>& rotate);
    RigidBody(const sp<Stub>& stub);

//  [[script::bindings::auto]]
    virtual void dispose() = 0;

//  [[script::bindings::property]]
    uint32_t id() const;
//  [[script::bindings::property]]
    Collider::BodyType type() const;

//  [[script::bindings::property]]
    V xy() const;
//  [[script::bindings::property]]
    V3 xyz() const;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;

//  [[script::bindings::property]]
    const sp<VV>& position() const;
//  [[script::bindings::property]]
    const sp<Size>& size() const;
//  [[script::bindings::property]]
    const sp<Rotate>& rotate() const;

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& box) const;

//  [[script::bindings::property]]
    const sp<CollisionCallback>& collisionCallback() const;
//  [[script::bindings::property]]
    void setCollisionCallback(const sp<CollisionCallback>& collisionCallback);

    const sp<Stub>& stub() const;

protected:
    sp<Stub> _stub;
};

}

#endif
