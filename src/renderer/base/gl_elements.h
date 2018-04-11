#ifndef ARK_RENDERER_GLES20_BASE_GL_ELEMENTS_H_
#define ARK_RENDERER_GLES20_BASE_GL_ELEMENTS_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLElements {
public:
    GLElements(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, const sp<ResourceLoaderContext>& resourceLoaderContext);

    sp<RenderCommand> render(const LayerContext::Snapshot& renderContext, float x, float y);
    sp<RenderCommand> renderInstanced(const LayerContext::Snapshot& renderContext, float x, float y);

private:
    sp<GLResourceManager> _resource_manager;
    sp<GLShader> _shader;
    sp<GLTexture> _texture;
    sp<GLModel> _model;
    GLenum _mode;

    GLBuffer _array_buffer;

    sp<ObjectPool> _render_command_pool;
    sp<MemoryPool> _memory_pool;
    sp<GLSnippetDelegate> _snippet;
};

}

#endif
