#pragma once

#include "core/forwarding.h"

#include "graphics/forwarding.h"

namespace ark {

class RendererMaker {
public:
    virtual ~RendererMaker() = default;

    virtual Vector<Box> make(float x, float y) = 0;
    virtual void recycle(const Box& renderer) = 0;

};

}
