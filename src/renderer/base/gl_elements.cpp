#include "renderer/base/gl_elements.h"

#include "core/base/object_pool.h"

#include "graphics/base/layer_context.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader_bindings.h"
#include "renderer/base/gl_snippet_delegate.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/impl/render_command/draw_elements_instanced.h"
#include "renderer/impl/gl_snippet/gl_snippet_active_texture.h"
#include "renderer/inf/gl_model.h"

namespace ark {

GLElementsRenderer::GLElementsRenderer(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, GLenum mode, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader), _texture(texture), _model(model), _mode(mode),
      _array_buffer(_resource_manager->createDynamicArrayBuffer()),
      _render_command_pool(resourceLoaderContext->objectPool()), _memory_pool(resourceLoaderContext->memoryPool()),
      _shader_bindings(sp<GLShaderBindings>::make(shader, _array_buffer))
{
    if(texture)
        _shader_bindings->snippet()->link<GLSnippetActiveTexture>(texture);
}

sp<RenderCommand> GLElementsRenderer::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    const GLBuffer indexBuffer = _model->getIndexBuffer(_resource_manager, renderContext);
    if(indexBuffer)
    {
        const array<uint8_t> buf = _model->getArrayBuffer(_memory_pool, renderContext, x, y);
        int32_t count = indexBuffer.length<glindex_t>();
        return _render_command_pool->obtain<DrawElements>(GLDrawingContext(_shader_bindings, _array_buffer.snapshot(buf), indexBuffer.snapshot(), _mode), _shader, count);
    }
    return nullptr;
}

}
