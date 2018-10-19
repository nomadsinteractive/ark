#ifndef ARK_RENDERER_INF_GL_MODEL_LOADER_H_
#define ARK_RENDERER_INF_GL_MODEL_LOADER_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/layer.h"

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

    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) = 0;
    virtual void loadModel(GLModelBuffer& buf, const Atlas& atlas, int32_t type, const V& size) = 0;

protected:
    GLenum _mode;
};

}

#endif
