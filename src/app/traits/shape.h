#pragma once

#include "core/forwarding.h"
#include "core/base/named_type.h"
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
    Shape(const NamedType& type = {Shape::TYPE_NONE}, sp<Vec3> size = nullptr);
    Shape(const NamedType& type, sp<Vec3> size, Box implementation);

//  [[script::bindings::property]]
    const NamedType& type() const;
//  [[script::bindings::property]]
    const SafeVar<Vec3>& size() const;

private:
    NamedType _type;
    SafeVar<Vec3> _size;

    Box _implementation;
};

}
