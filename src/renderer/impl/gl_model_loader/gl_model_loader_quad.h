#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_QUAD_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_QUAD_H_

#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderQuad : public GLModelLoader {
public:
    GLModelLoaderQuad(const sp<Atlas>& atlas);

    virtual uint32_t estimateVertexCount(uint32_t renderObjectCount) override;
    virtual void loadVertices(GLModelBuffer& buf, uint32_t type, const V& scale) override;

    virtual GLBuffer getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount) override;

//  [[plugin::builder("quad")]]
    class BUILDER : public Builder<GLModelLoader> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<GLModelLoader> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Atlas>> _atlas;
    };

private:
    sp<Atlas> _atlas;

};

}

#endif
