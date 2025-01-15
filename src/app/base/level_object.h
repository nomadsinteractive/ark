#pragma once

#include "core/base/api.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "app/components/rigidbody.h"

namespace ark {

class ARK_API LevelObject {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_INSTANCE,
        TYPE_MESH,
        TYPE_CAMERA,
        TYPE_LIGHT
    };

    LevelObject(document manifest);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(LevelObject);

//  [[script::bindings::property]]
    const document& manifest() const;
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
    const sp<Entity>& entity() const;
//  [[script::bindings::property]]
    void setEntity(sp<Entity> entity);

//  [[script::bindings::property]]
    const sp<RenderObject>& renderObject() const;

private:
    document _manifest;

    String _name;
    Type _type;
    bool _visible;
    V3 _position;
    Optional<V3> _scale;
    Optional<V4> _rotation;
    Rigidbody::BodyType _body_type;
    int32_t _instance_of;

    sp<Entity> _entity;
    sp<RenderObject> _render_object;
    sp<Rigidbody> _rigidbody;

    friend class Level;
    friend class LevelLayer;
};

}
