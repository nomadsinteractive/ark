#ifndef ARK_GRAPHICS_INF_RENDER_COMMAND_H_
#define ARK_GRAPHICS_INF_RENDER_COMMAND_H_

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

#endif
