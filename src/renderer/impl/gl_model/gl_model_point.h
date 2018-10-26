#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_POINT_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_POINT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model.h"

namespace ark {

class GLModelPoint : public GLModel {
public:
    GLModelPoint(const sp<Atlas>& atlas);

    virtual void initialize(GLShaderBindings& bindings) override;
    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) override;
    virtual void load(GLModelBuffer& buf, int32_t type, const V& scale) override;

//  [[plugin::builder("point")]]
    class BUILDER : public Builder<GLModel> {
    public:
        BUILDER(BeanFactory& factory, const document manifest);

        virtual sp<GLModel> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;
};

}

#endif
