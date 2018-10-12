#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_LINE_STRIP_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_LINE_STRIP_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderLineStrip : public GLModelLoader {
public:
    GLModelLoaderLineStrip(const sp<ResourceLoaderContext>& resourceLoaderContext);

    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const LayerContext::Snapshot& layerContext) override;
    virtual void loadModel(GLModelBuffer& buf, const Atlas& atlas, int32_t type, const V& scale) override;

//  [[plugin::resource-loader::by-value("line-strip")]]
    class BUILDER : public Builder<GLModelLoader> {
    public:
        BUILDER(const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLModelLoader> build(const sp<Scope>& args) override;

    private:
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    GLBuffer _ibo;
};

}

#endif
