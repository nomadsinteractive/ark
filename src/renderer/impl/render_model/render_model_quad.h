#ifndef ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_QUAD_H_
#define ARK_RENDERER_IMPL_RENDER_MODEL_RENDER_MODEL_QUAD_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"

namespace ark {

class GLModelQuad : public RenderModel {
public:
    GLModelQuad(const sp<Atlas>& atlas);

    virtual sp<ShaderBindings> makeShaderBindings(const RenderController& renderController, const sp<PipelineLayout>& pipelineLayout) override;

    virtual void start(ModelBuffer& buf, RenderController& renderController, const Layer::Snapshot& layerContext) override;
    virtual void load(ModelBuffer& buf, int32_t type, const V& scale) override;
    virtual Metrics measure(int32_t type) override;

//  [[plugin::builder("quad")]]
    class BUILDER : public Builder<RenderModel> {
    public:
        BUILDER(BeanFactory& factory, const document manifest);

        virtual sp<RenderModel> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;
};

}

#endif