#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_POINT_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_POINT_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderPoint : public GLModelLoader {
public:
    GLModelLoaderPoint();

    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext) override;
    virtual void loadModel(GLModelBuffer& buf, const Atlas& atlas, uint32_t type, const V& scale) override;

//  [[plugin::builder::by-value("point")]]
    class BUILDER : public Builder<GLModelLoader> {
    public:
        BUILDER();

        virtual sp<GLModelLoader> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;
    };

private:
    sp<GLResourceManager> _resource_manager;
};

}

#endif
