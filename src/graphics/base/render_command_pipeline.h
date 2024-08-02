#pragma once

#include <vector>

#include "core/forwarding.h"

#include "graphics/inf/render_command.h"
#include "graphics/forwarding.h"

namespace ark {

class RenderCommandPipeline final : public RenderCommand {
public:

    void add(sp<RenderCommand> renderCommand);

    void draw(GraphicsContext& graphicsContext) override;

private:
    std::vector<sp<RenderCommand>> _commands;
};

}
