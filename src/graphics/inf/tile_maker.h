#pragma once

#include <vector>

#include "graphics/forwarding.h"

namespace ark {

class RendererMaker {
public:
    virtual ~RendererMaker() = default;

    virtual std::vector<Box> make(float x, float y) = 0;
    virtual void recycle(const Box& renderer) = 0;

};

}
