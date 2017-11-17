#ifndef ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_POINT_H_
#define ARK_RENDERER_IMPL_GL_MODEL_GL_MODEL_POINT_H_

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/transform.h"

#include "renderer/forwarding.h"
#include "renderer/inf/gl_model.h"

namespace ark {

class GLModelPoint : public GLModel {
public:
    GLModelPoint(const sp<GLShader>& shader, const sp<Atlas>& texCoords);

    virtual array<uint8_t> getArrayBuffer(GLResourceManager& resourceManager, const LayerContext::Snapshot& renderContext, float x, float y) override;
    virtual GLBuffer getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext) override;

    virtual uint32_t mode() const override;

private:
    void map(uint8_t*& buf, const Transform::Snapshot& snapshot, float x, float tx, float y, float ty, uint16_t texCoordX, uint16_t texCoordY) const;

private:
    sp<Atlas> _atlas;
    uint32_t _tex_coordinate_offset;
    uint32_t _stride;
};

}

#endif
