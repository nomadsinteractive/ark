#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_QUAD_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_QUAD_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/render_model.h"

namespace ark {

class GLModelQuad : public RenderModel {
public:
    GLModelQuad(const sp<Atlas>& atlas);

    virtual void initialize(ShaderBindings& bindings) override;
    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) override;
    virtual void load(GLModelBuffer& buf, int32_t type, const V& scale) override;
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
