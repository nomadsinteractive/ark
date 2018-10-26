#ifndef ARK_RENDERER_INF_GL_MODEL_H_
#define ARK_RENDERER_INF_GL_MODEL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer.h"
#include "graphics/base/metrics.h"

#include "renderer/forwarding.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLModel {
public:
    GLModel(GLenum mode)
        : _mode(mode) {
    }
    virtual ~GLModel() = default;

    GLenum mode() const {
        return _mode;
    }

    virtual void initialize(GLShaderBindings& bindings) = 0;
    virtual void start(GLModelBuffer& buf, GLResourceManager& resourceManager, const Layer::Snapshot& layerContext) = 0;
    virtual void load(GLModelBuffer& buf, int32_t type, const V& size) = 0;

    virtual Metrics measure(int32_t type) { return Metrics(); }

protected:
    GLenum _mode;
};

}

#endif
