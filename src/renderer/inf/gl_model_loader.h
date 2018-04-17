#ifndef ARK_RENDERER_INF_GL_MODEL_LOADER_H_
#define ARK_RENDERER_INF_GL_MODEL_LOADER_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLModelLoader {
public:
    GLModelLoader(GLenum mode)
        : _mode(mode) {
    }
    virtual ~GLModelLoader() = default;

    GLenum mode() const {
        return _mode;
    }

    virtual uint32_t estimateVertexCount(uint32_t renderObjectCount) = 0;

    virtual void loadVertices(GLModelBuffer& buf, uint32_t type, const V& size) = 0;

    virtual GLBuffer getPredefinedIndexBuffer(GLResourceManager& glResourceManager, uint32_t renderObjectCount) {
        return GLBuffer();
    }

private:
    GLenum _mode;
};

}

#endif
