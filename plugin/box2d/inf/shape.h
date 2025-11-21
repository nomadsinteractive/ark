#pragma once

#include <Box2D/Box2D.h>

#include "graphics/components/size.h"

#include "box2d/forwarding.h"

namespace ark::plugin::box2d {

class Shape {
public:
    virtual ~Shape() = default;

    virtual void apply(b2BodyId body, const V3& size, const BodyCreateInfo& createInfo) = 0;

};

}
