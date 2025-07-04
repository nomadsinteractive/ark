#pragma once

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/render_command.h"
#include "graphics/forwarding.h"

namespace ark {

class RenderCommandPipeline final : public RenderCommand {
public:

    void add(sp<RenderCommand> renderCommand);

    void draw(GraphicsContext& graphicsContext) override;

private:
    Vector<sp<RenderCommand>> _commands;
};

}
