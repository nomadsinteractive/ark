#pragma once

#include "core/forwarding.h"
#include "core/base/named_hash.h"
#include "core/types/optional_var.h"

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
    Shape(const NamedHash& type = {Shape::TYPE_NONE}, Optional<V3> scale = {}, sp<Vec3> origin = nullptr);
    Shape(const NamedHash& type, Optional<V3> scale, sp<Vec3> orgin, Box implementation);

//  [[script::bindings::property]]
    const NamedHash& type() const;
//  [[script::bindings::property]]
    const Optional<V3>& scale() const;
//  [[script::bindings::property]]
    const OptionalVar<Vec3>& origin() const;

    const Box& implementation() const;
    void setImplementation(Box implementation);

    template<typename T> sp<T> asImplementation() const {
        if(!_implementation)
            return nullptr;

        sp<T> implementation = _implementation.as<T>();
        ASSERT(implementation);
        return implementation;
    }

private:
    NamedHash _type;
    Optional<V3> _scale;
    OptionalVar<Vec3> _origin;

    Box _implementation;
};

}
