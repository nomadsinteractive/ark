#ifndef ARK_RENDERER_IMPL_RENDERER_FRAMEBUFFER_RENDERER_H_
#define ARK_RENDERER_IMPL_RENDERER_FRAMEBUFFER_RENDERER_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

#include "renderer/opengl/forwarding.h"

namespace ark {
namespace opengl {

class GLFramebufferRenderer : public Renderer {
public:
    GLFramebufferRenderer(const sp<Renderer>& delegate, const sp<GLFramebuffer>& framebuffer);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

private:
    sp<Renderer> _delegate;
    sp<opengl::GLFramebuffer> _fbo;
    sp<RenderCommand> _pre_draw;
    sp<RenderCommand> _post_draw;

};

}
}

#endif
