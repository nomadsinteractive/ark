#ifndef ARK_GRAPHICS_BASE_RENDER_COMMAND_PIPELINE_H_
#define ARK_GRAPHICS_BASE_RENDER_COMMAND_PIPELINE_H_

#include <vector>

#include "core/forwarding.h"

#include "graphics/inf/render_command.h"
#include "graphics/forwarding.h"

namespace ark {

class RenderCommandPipeline : public RenderCommand {
public:

    void add(const sp<RenderCommand>& renderCommand);

    virtual void draw(GraphicsContext& graphicsContext) override;

private:
    std::vector<sp<RenderCommand>> _commands;
};

}

#endif
