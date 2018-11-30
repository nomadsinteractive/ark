#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_DRAW_ELEMENTS_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_DRAW_ELEMENTS_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/drawing_context.h"
#include "renderer/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {
namespace opengl {

class ARK_API DrawElements : public RenderCommand {
public:
    DrawElements(DrawingContext context, const sp<Shader>& shader, GLenum mode);

    virtual void draw(GraphicsContext& graphicsContext) override;

private:
    DrawingContext _context;
    sp<Shader> _shader;
    GLenum _mode;
};

}
}

#endif
