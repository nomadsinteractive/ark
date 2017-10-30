#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_DRAW_ELEMENTS_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_DRAW_ELEMENTS_H_

#include "core/types/shared_ptr.h"

#include "graphics/inf/render_command.h"

#include "renderer/base/gl_snippet_context.h"
#include "renderer/forwarding.h"

namespace ark {

class DrawElements : public RenderCommand {
public:
    DrawElements(const GLSnippetContext& context, const sp<GLShader>& shader, const sp<GLSnippet>& procedure);

    virtual void draw(const op<GraphicsContext>& graphicsContext) override;

private:
    GLSnippetContext _context;
    sp<GLShader> _shader;
    sp<GLSnippet> _snippet;
};

}

#endif
