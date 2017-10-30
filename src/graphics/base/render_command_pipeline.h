#ifndef ARK_GRAPHICS_BASE_RENDER_COMMAND_PIPELINE_H_
#define ARK_GRAPHICS_BASE_RENDER_COMMAND_PIPELINE_H_

#include <list>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/render_command.h"
#include "graphics/forwarding.h"

namespace ark {

class RenderCommandPipeline : public RenderCommand {
public:

    void reset();
    void add(const sp<RenderCommand>& renderCommand);

    virtual void draw(const op<GraphicsContext>& graphicsContext) override;

private:
    std::list<sp<RenderCommand>> _commands;
};

}

#endif
