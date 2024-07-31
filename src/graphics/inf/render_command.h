#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API RenderCommand {
public:
    virtual ~RenderCommand() = default;

    virtual void draw(GraphicsContext& graphicsContext) = 0;

};

}
