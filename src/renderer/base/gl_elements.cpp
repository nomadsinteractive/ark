#include "renderer/base/gl_elements.h"

#include "core/base/object_pool.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/inf/gl_model.h"

namespace ark {

GLElements::GLElements(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader), _texture(texture), _model(model), _mode(static_cast<GLenum>(model->mode())),
      _array_buffer(_resource_manager->createGLBuffer(nullptr, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW)),
      _render_command_pool(resourceLoaderContext->objectPool()), _memory_pool(resourceLoaderContext->memoryPool()),
      _gl_snippet(_resource_manager->createCoreGLSnippet(_shader, _array_buffer))
{
}

sp<RenderCommand> GLElements::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    const GLBuffer indexBuffer = _model->getIndexBuffer(_resource_manager, renderContext);
    if(indexBuffer)
    {
        const array<uint8_t> buf = _model->getArrayBuffer(_memory_pool, renderContext, x, y);
        return _render_command_pool->obtain<DrawElements>(GLSnippetContext(_texture, _array_buffer.snapshot(buf), indexBuffer, _mode), _shader, _gl_snippet);
    }
    return nullptr;
}

}
