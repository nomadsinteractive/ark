#ifndef ARK_APP_BASE_RIGID_BODY_H_
#define ARK_APP_BASE_RIGID_BODY_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/types/box.h"
#include "core/types/weak_ptr.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

#include "app/forwarding.h"
#include "app/inf/collider.h"

namespace ark {

//[[script::bindings::holder]]
class ARK_API RigidBody : public Holder {
public:
    class ARK_API Callback {
    public:
        void onBeginContact(const sp<RigidBody>& rigidBody, const CollisionManifold& manifold);
        void onEndContact(const sp<RigidBody>& rigidBody);

        void onBeginContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody, const CollisionManifold& manifold);
        void onEndContact(const sp<RigidBody>& self, const sp<RigidBody>& rigidBody);

        bool hasCallback() const;

    private:
        sp<CollisionCallback> _collision_callback;

        friend class RigidBody;
    };

    struct ARK_API Stub {
        Stub(int32_t id, Collider::BodyType type, int32_t shapeId, sp<Vec3> position, sp<Size> size, sp<Transform> transform, Box impl, sp<Disposed> disposed = nullptr);
        ~Stub();

        DISALLOW_COPY_AND_ASSIGN(Stub);

        int32_t _id;
        Collider::BodyType _type;
        int32_t _shape_id;
        SafePtr<Vec3> _position;
        SafePtr<Size> _size;
        sp<Transform> _transform;

        Box _impl;
        SafePtr<Disposed> _disposed;

        sp<Callback> _callback;
        WeakPtr<RenderObject> _render_object;

        Box _tag;
    };

public:
    virtual ~RigidBody() override = default;

    RigidBody(int32_t id, Collider::BodyType type, int32_t shapeId, sp<Vec3> position, sp<Size> size, sp<Rotation> rotate, Box impl, sp<Disposed> disposed);
    RigidBody(sp<Stub> stub);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(RigidBody);

//  [[script::bindings::auto]]
    virtual void dispose() = 0;
//  [[script::bindings::auto]]
    virtual void bind(const sp<RenderObject>& renderObject);

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    int32_t id() const;
//  [[script::bindings::property]]
    Collider::BodyType type() const;
//  [[script::bindings::property]]
    int32_t shapeId() const;

//  [[script::bindings::property]]
    V2 xy() const;
//  [[script::bindings::property]]
    V3 xyz() const;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    float height() const;

//  [[script::bindings::property]]
    const sp<Vec3>& position() const;
//  [[script::bindings::property]]
    const sp<Size>& size() const;
//  [[script::bindings::property]]
    const sp<Transform>& transform() const;

//  [[script::bindings::property]]
    const Box& tag() const;
//  [[script::bindings::property]]
    void setTag(const Box& box) const;

//  [[script::bindings::property]]
    sp<RenderObject> renderObject() const;

//  [[script::bindings::property]]
    const sp<Disposed>& disposed() const;

//  [[script::bindings::property]]
    const sp<CollisionCallback>& collisionCallback() const;
//  [[script::bindings::property]]
    void setCollisionCallback(const sp<CollisionCallback>& collisionCallback);

    const sp<Stub>& stub() const;
    const sp<Callback>& callback() const;

//  [[plugin::style("rigid-body")]]
    class RIGID_BODY_STYLE : public Builder<RenderObject> {
    public:
        RIGID_BODY_STYLE(BeanFactory& factory, const sp<Builder<RenderObject>>& delegate, const String& value);

        virtual sp<RenderObject> build(const Scope& args) override;

    private:
        sp<Builder<RenderObject>> _delegate;
        sp<Builder<RigidBody>> _rigid_body;

    };

private:
    sp<Stub> _stub;

};

}

#endif
