#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_DRAW_ELEMENTS_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_DRAW_ELEMENTS_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/gl_drawing_context.h"
#include "renderer/forwarding.h"

namespace ark {

class ARK_API DrawElements : public RenderCommand {
public:
    DrawElements(GLDrawingContext context, const sp<GLPipeline>& shader);

    virtual void draw(GraphicsContext& graphicsContext) override;

private:
    GLDrawingContext _context;
    sp<GLPipeline> _shader;
};

}

#endif
