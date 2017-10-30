#ifndef ARK_RENDERER_IMPL_RENDERER_FRAMEBUFFER_RENDERER_H_
#define ARK_RENDERER_IMPL_RENDERER_FRAMEBUFFER_RENDERER_H_

#include "core/base/object_pool.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"

#include "renderer/forwarding.h"

namespace ark {

class FrameBufferRenderer : public Renderer {
public:
    FrameBufferRenderer(const sp<Renderer>& delegate, const sp<GLTexture>& texture, const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override;

//  [[plugin::resource-loader("framebuffer")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Renderer>> _delegate;
        sp<Builder<GLTexture>> _texture;
    };

private:
    sp<Renderer> _delegate;
    ObjectPool<RenderCommand> _render_commands_pool;

    sp<GLFramebuffer> _fbo;

};

}

#endif
