#ifndef ARK_RENDERER_BASE_GL_ELEMENTS_H_
#define ARK_RENDERER_BASE_GL_ELEMENTS_H_

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"

#include "renderer/forwarding.h"
#include "renderer/base/gl_buffer.h"

#include "platform/gl/gl.h"

namespace ark {

class ARK_API GLElementsRenderer {
public:
    GLElementsRenderer(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, GLenum mode, const sp<ResourceLoaderContext>& resourceLoaderContext);

    sp<RenderCommand> render(const LayerContext::Snapshot& renderContext, float x, float y);

private:
    sp<GLResourceManager> _resource_manager;
    sp<GLShader> _shader;
    sp<GLTexture> _texture;
    sp<GLModel> _model;
    GLenum _mode;

    GLBuffer _array_buffer;

    sp<ObjectPool> _render_command_pool;
    sp<MemoryPool> _memory_pool;
    sp<GLShaderBindings> _shader_bindings;
};

}

#endif
