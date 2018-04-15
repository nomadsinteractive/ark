#ifndef ARK_RENDERER_INF_GL_MODEL_H_
#define ARK_RENDERER_INF_GL_MODEL_H_

#include "core/base/api.h"
#include "core/forwarding.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

namespace ark {

class ARK_API GLModel {
public:
    virtual ~GLModel() = default;

    virtual bytearray getArrayBuffer(MemoryPool& memoryPool, const LayerContext::Snapshot& renderContext, float x, float y) = 0;
    virtual GLBuffer getIndexBuffer(GLResourceManager& glResourceManager, const LayerContext::Snapshot& renderContext) = 0;
};

}

#endif
