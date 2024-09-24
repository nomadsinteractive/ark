#pragma once

#include "core/forwarding.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Shape final {
public:
//  [[script::bindings::enumeration]]
    enum Type {
        TYPE_NONE = 0,
        TYPE_AABB = string_hash("aabb"),
        TYPE_BALL = string_hash("ball"),
        TYPE_BOX = string_hash("box"),
        TYPE_CAPSULE = string_hash("capsule")
    };

public:
//  [[script::bindings::auto]]
    Shape(TypeId type = Shape::TYPE_NONE, sp<Vec3> size = nullptr);

//  [[script::bindings::property]]
    TypeId type() const;
//  [[script::bindings::property]]
    void setType(TypeId type);
//  [[script::bindings::property]]
    void setType(const String& type);
//  [[script::bindings::property]]
    const SafeVar<Vec3>& size() const;
//  [[script::bindings::property]]
    void setSize(sp<Vec3> size);

private:
    TypeId _type;
    SafeVar<Vec3> _size;
};

}
