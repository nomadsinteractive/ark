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
    GLFramebufferRenderer(sp<Renderer> delegate, sp<GLFramebuffer> framebuffer, int32_t clearMask);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

private:
    sp<Renderer> _delegate;
    sp<opengl::GLFramebuffer> _fbo;
    sp<RenderCommand> _pre_draw;
    sp<RenderCommand> _post_draw;

};

}
}

#endif
