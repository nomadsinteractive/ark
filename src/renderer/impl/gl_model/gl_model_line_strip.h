#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_LINE_STRIP_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_LINE_STRIP_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"
#include "renderer/inf/gl_model.h"

namespace ark {

class GLModelLineStrip : public GLModel {
public:
    GLModelLineStrip(const sp<ResourceLoaderContext>& resourceLoaderContext, const sp<Atlas>& atlas);

    virtual void initialize(GLShaderBindings& bindings) override;
    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) override;
    virtual void load(GLModelBuffer& buf, int32_t type, const V& scale) override;

//  [[plugin::resource-loader("line-strip")]]
    class BUILDER : public Builder<GLModel> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext);

        virtual sp<GLModel> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
        sp<ResourceLoaderContext> _resource_loader_context;

    };

private:
    std::vector<glindex_t> makeIndices(const Layer::Snapshot& layerContext);

private:
    sp<Atlas> _atlas;

    GLBuffer _ibo;
};

}

#endif
