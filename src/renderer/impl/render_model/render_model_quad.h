#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_QUAD_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_QUAD_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"
#include "renderer/base/render_controller.h"

namespace ark {

class GLModelQuad : public RenderModel {
public:
    GLModelQuad(const RenderController& renderController, const sp<Atlas>& atlas);

    virtual sp<ShaderBindings> makeShaderBindings(const Shader& shader) override;

    virtual void start(DrawingBuffer& buf, const RenderLayer::Snapshot& layerContext) override;
    virtual void load(DrawingBuffer& buf, const RenderObject::Snapshot& snapshot) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual Metrics measure(int32_t type) override;

//  [[plugin::resource-loader("quad")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    sp<Atlas> _atlas;
    sp<NamedBuffer> _index_buffer;
};

}

#endif
