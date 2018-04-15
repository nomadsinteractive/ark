#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_LINE_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_LINE_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model.h"

namespace ark {

class GLModelLine : public GLModel {
public:
    GLModelLine(const sp<GLShader>& shader, const sp<Atlas>& atlas);

    virtual bytearray getArrayBuffer(MemoryPool& memoryPool, const LayerContext::Snapshot& renderContext, float x, float y) override;
    virtual GLBuffer getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext) override;

private:
    sp<Atlas> _atlas;
    uint32_t _tex_coordinate_offset;
    uint32_t _stride;

};

}

#endif
