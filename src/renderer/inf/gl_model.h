#ifndef ARK_RENDERER_INF_GL_MODEL_H_
#define ARK_RENDERER_INF_GL_MODEL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"

#include "renderer/forwarding.h"

namespace ark {

class ARK_API GLModel {
public:
    virtual ~GLModel() = default;

    virtual array<uint8_t> getArrayBuffer(GLResourceManager& resourceManager, const LayerContext& renderContext, float x, float y) = 0;
    virtual GLBuffer getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext& renderContext) = 0;

    virtual uint32_t mode() const = 0;
};

}

#endif
