#pragma once

#include <Box2D/Box2D.h>

#include "graphics/traits/size.h"

#include "box2d/forwarding.h"

namespace ark::plugin::box2d {

//[[script::bindings::auto]]
class Shape {
public:
    virtual ~Shape() = default;

    virtual void apply(b2Body* body, const V3& size, const BodyCreateInfo& createInfo) = 0;

};

}
