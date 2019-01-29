#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_POINT_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_POINT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/render_controller.h"
#include "renderer/inf/render_model.h"

namespace ark {

class GLModelPoint : public RenderModel {
public:
    GLModelPoint(const RenderController& renderController, const sp<Atlas>& atlas);

    virtual sp<ShaderBindings> makeShaderBindings(RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) override;
    virtual void postSnapshot(RenderController& renderController, Layer::Snapshot& snapshot) override;

    virtual void start(ModelBuffer& buf, const Layer::Snapshot& layerContext) override;
    virtual void load(ModelBuffer& buf, const RenderObject::Snapshot& snapshot) override;

//  [[plugin::resource-loader("point")]]
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
