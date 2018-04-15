#ifndef ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_QUAD_H_
#define ARK_RENDERER_IMPL_GL_MODEL_LOADER_GL_MODEL_LOADER_QUAD_H_

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model_loader.h"

namespace ark {

class GLModelLoaderQuad : public GLModelLoader {
public:
    GLModelLoaderQuad(const sp<Atlas>& atlas);

    virtual uint32_t estimateVertexCount(uint32_t renderObjectCount) override;
    virtual void load(GLElementsBuffer& buf, uint32_t type, const V& scale) override;

    virtual GLBuffer getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount) override;

private:
    sp<Atlas> _atlas;

};

}

#endif
