#ifndef ARK_RENDERER_IMPL_RENDERER_RENDER_PASS_H_
#define ARK_RENDERER_IMPL_RENDERER_RENDER_PASS_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"

namespace ark {

class RenderPass : public Renderer {
public:
    RenderPass(sp<Shader> shader, sp<Buffer> vertexBuffer, sp<Integer> drawCount);

    virtual void render(RenderRequest& renderRequest, const V3& position) override;

//  [[plugin::resource-loader("render-pass")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

        sp<Builder<Shader>> _shader;

        sp<Builder<Buffer>> _vertex_buffer;
        sp<Builder<Integer>> _draw_count;
    };

private:
    sp<Shader> _shader;
    sp<Buffer> _vertex_buffer;
    sp<Integer> _draw_count;

    sp<ShaderBindings> _shader_bindings;
    Buffer::Snapshot _index_buffer;
};

}

#endif
