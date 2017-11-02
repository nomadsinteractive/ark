#include "renderer/base/gl_elements.h"

#include "core/base/object_pool.h"
#include "core/types/global.h"

#include "graphics/base/layer_context.h"
#include "graphics/base/render_command_pipeline.h"

#include "renderer/base/gl_resource_manager.h"
#include "renderer/base/gl_shader.h"
#include "renderer/base/render_engine.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"
#include "renderer/impl/render_command/draw_elements.h"
#include "renderer/inf/gl_model.h"

namespace ark {

GLElements::GLElements(const sp<GLShader>& shader, const sp<GLTexture>& texture, const sp<GLModel>& model, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_manager(resourceLoaderContext->glResourceManager()), _shader(shader), _texture(texture), _model(model), _mode(static_cast<GLenum>(model->mode()))
    , _array_buffer(_resource_manager->createGLBuffer(nullptr, GL_ARRAY_BUFFER, GL_DYNAMIC_DRAW))
    , _render_command_pool(resourceLoaderContext->getObjectPool<RenderCommand>()), _gl_snippet(createCoreGLSnippet(shader->snippet()))
{
}

void GLElements::render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y)
{
    const GLBuffer indexBuffer = _model->getIndexBuffer(_resource_manager, renderContext);
    if(indexBuffer)
    {
        const array<uint8_t> buf = _model->getArrayBuffer(_resource_manager, renderContext, x, y);
        pipeline.add(_render_command_pool->allocate<DrawElements>(GLSnippetContext(_texture, _array_buffer.snapshot(buf), indexBuffer, _mode), _shader, _gl_snippet));
    }
}

sp<GLSnippet> GLElements::createCoreGLSnippet(const sp<GLSnippet>& glSnippet) const
{
    return sp<GLSnippetWrapper>::make(_resource_manager, _shader, _array_buffer, glSnippet);
}

GLElements::CoreGLSnippet::CoreGLSnippet(GLElements::GLSnippetWrapper& wrapper, const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix)
    : _wrapper(wrapper), _gl_resource_manager(glResourceManager), _shader(shader), _array_buffer(arrayBuffer), _appendix(appendix)
{
}

void GLElements::CoreGLSnippet::preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context)
{
    const sp<GLSnippet> delegate = _wrapper._delegate;
    _wrapper._delegate = createGLSnippet();
    _wrapper.preCompile(source, context);
}

void GLElements::CoreGLSnippet::preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLSnippetContext& context)
{
    const sp<GLSnippet> delegate = _wrapper._delegate;
    _wrapper._delegate = createGLSnippet();
    _wrapper.preDraw(graphicsContext, shader, context);
}

void GLElements::CoreGLSnippet::postDraw(GraphicsContext& graphicsContext)
{
    const sp<GLSnippet> delegate = _wrapper._delegate;
    _wrapper._delegate = createGLSnippet();
    _wrapper.postDraw(graphicsContext);
}

sp<GLSnippet> GLElements::CoreGLSnippet::createGLSnippet() const
{
    const Global<RenderEngine> renderEngine;
    const sp<GLSnippet> coreGLSnippet = renderEngine->createCoreGLSnippet(_gl_resource_manager, _shader, _array_buffer);
    return _appendix ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(coreGLSnippet, _appendix)) : coreGLSnippet;
}

GLElements::GLSnippetWrapper::GLSnippetWrapper(const sp<GLResourceManager>& glResourceManager, const sp<GLShader>& shader, const GLBuffer& arrayBuffer, const sp<GLSnippet>& appendix)
    : _delegate(sp<CoreGLSnippet>::make(*this, glResourceManager, shader, arrayBuffer, appendix))
{
}

void GLElements::GLSnippetWrapper::preCompile(GLShaderSource& source, GLShaderPreprocessor::Context& context)
{
    _delegate->preCompile(source, context);
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
