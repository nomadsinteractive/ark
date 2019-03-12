#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_LINE_STRIP_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_LINE_STRIP_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/inf/render_model.h"

namespace ark {

class GLModelLineStrip : public RenderModel {
public:
    GLModelLineStrip(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas);

    virtual sp<ShaderBindings> makeShaderBindings(RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) override;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;

    virtual void start(ModelBuffer& buf, const RenderLayer::Snapshot& snapshot) override;
    virtual void load(ModelBuffer& buf, const RenderObject::Snapshot& snapshot) override;

//  [[plugin::resource-loader("line-strip")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    std::vector<glindex_t> makeIndices(const RenderLayer::Snapshot& layerContext);

private:
    sp<Atlas> _atlas;

    Buffer _ibo;

    size_t _last_rendered_count;
    Buffer::Snapshot _indices;
};

}

#endif
