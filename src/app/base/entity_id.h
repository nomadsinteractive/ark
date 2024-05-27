#pragma once

#include "core/base/api.h"

#include "app/base/entity.h"

namespace ark {

class ARK_API EntityId {
public:
    EntityId(const Entity& entity);

//  [[script::bindings::operator(index)]]
    static uintptr_t toInteger(const EntityId& self);

private:
    sp<Entity::Ref> _ref;

};

}