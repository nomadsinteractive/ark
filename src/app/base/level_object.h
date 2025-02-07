#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v4.h"

#include "app/base/level.h"
#include "app/components/rigidbody.h"

namespace ark {

class ARK_API LevelObject {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_INSTANCE,
        TYPE_ELEMENT,
        TYPE_CAMERA,
        TYPE_LIGHT
    };

    LevelObject(const sp<Level::Stub>& level, const document& manifest);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LevelObject);

//  [[script::bindings::property]]
    const String& name() const;
//  [[script::bindings::property]]
    LevelObject::Type type() const;
//  [[script::bindings::property]]
    bool visible() const;
//  [[script::bindings::property]]
    const V3& position() const;
//  [[script::bindings::property]]
    const Optional<V3>& scale() const;
//  [[script::bindings::property]]
    const Optional<V4>& rotation() const;
//  [[script::bindings::property]]
    int32_t instanceOf() const;
//  [[script::bindings::property]]
    sp<Shape> shape() const;

//  [[script::bindings::property]]
    const sp<RenderObject>& renderObject() const;
//  [[script::bindings::property]]
    const sp<Rigidbody>& rigidbody() const;

//  [[script::bindings::auto]]
    sp<RenderObject> createRenderObject();
//  [[script::bindings::auto]]
    sp<Rigidbody> createRigidbody(const sp<Collider>& collider, Rigidbody::BodyType bodyType, const Map<String, sp<Shape>>& shapes, const sp<CollisionFilter>& collisionFilter);

private:

    const sp<LevelLibrary>& library() const;

private:
    sp<Level::Stub> _level;
    String _name;
    Type _type;
    bool _visible;
    V3 _position;
    Optional<V3> _scale;
    Optional<V4> _rotation;
    int32_t _instance_of;

    sp<Entity> _entity;
    sp<RenderObject> _render_object;
    sp<Rigidbody> _rigidbody;

    friend class Level;
    friend class LevelLayer;
};

}
