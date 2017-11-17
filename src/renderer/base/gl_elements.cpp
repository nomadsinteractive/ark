#include "renderer/base/gl_elements.h"

#include "core/base/object_pool.h"
#include "core/types/global.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/gl_context.h"
#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/inf/gl_model.h"

namespace ark {

GLElements::GLElements(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader), _texture(texture), _model(model), _mode(static_cast<GLenum>(model->mode())),
      _array_buffer(_resource_manager->createGLBuffer(nullptr, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW)),
      _render_command_pool(resourceLoaderContext->objectPool()),
      _gl_snippet(sp<GLSnippetWrapper>::make(_resource_manager, _shader, _array_buffer, shader->snippet()))
{
}

sp<RenderCommand> GLElements::render(const LayerContext::Snapshot& renderContext, float x, float y)
{
    const GLBuffer indexBuffer = _model->getIndexBuffer(_resource_manager, renderContext);
    if(indexBuffer)
    {
        const array<uint8_t> buf = _model->getArrayBuffer(_resource_manager, renderContext, x, y);
        return _render_command_pool->allocate<DrawElements>(GLSnippetContext(_texture, _array_buffer.snapshot(buf), indexBuffer, _mode), _shader, _gl_snippet);
    }
    return nullptr;
}

GLElements::CoreGLSnippet::CoreGLSnippet(GLSnippetWrapper& wrapper, const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix)
    : _wrapper(wrapper), _delegate(appendix), _gl_resource_manager(glResourceManager), _shader(shader), _array_buffer(arrayBuffer)
{
}

void GLElements::CoreGLSnippet::preInitialize(GLShaderSource& source)
{
    _wrapper.preInitialize(source);
}

void GLElements::CoreGLSnippet::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context)
{
    const sp<GLSnippet> delegate = _wrapper._delegate;
    _wrapper._delegate = createGLSnippet(graphicsContext);
    _wrapper.preCompile(graphicsContext, context);
}

void GLElements::CoreGLSnippet::preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context)
{
    const sp<GLSnippet> delegate = _wrapper._delegate;
    _wrapper._delegate = createGLSnippet(graphicsContext);
    _wrapper.preDraw(graphicsContext, shader, context);
}

void GLElements::CoreGLSnippet::postDraw(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("You're not supposed to be here");
}

sp<GLSnippet> GLElements::CoreGLSnippet::createGLSnippet(GraphicsContext& graphicsContext) const
{
    const sp<GLSnippet> coreGLSnippet = graphicsContext.glContext()->createCoreGLSnippet(_gl_resource_manager, _shader, _array_buffer);
    return _delegate ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(coreGLSnippet, _delegate)) : coreGLSnippet;
}

GLElements::GLSnippetWrapper::GLSnippetWrapper(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix)
    : _delegate(sp<CoreGLSnippet>::make(*this, glResourceManager, shader, arrayBuffer, appendix))
{
}

void GLElements::GLSnippetWrapper::preInitialize(GLShaderSource& source)
{
    _delegate->preInitialize(source);
}

void GLElements::GLSnippetWrapper::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context)
{
    _delegate->preCompile(graphicsContext, context);
}

void GLElements::GLSnippetWrapper::preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context)
{
    _delegate->preDraw(graphicsContext, shader, context);
}

void GLElements::GLSnippetWrapper::postDraw(GraphicsContext& graphicsContext)
{
    _delegate->postDraw(graphicsContext);
}

}
