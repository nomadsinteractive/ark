#include "renderer/base/gl_snippet_delegate.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"

namespace ark {

class CoreGLSnippet : public GLSnippet {
public:
    CoreGLSnippet(GLSnippetDelegate& wrapper, const sp<GLShader>& shader, const GLBuffer& arrayBuffer);

    virtual void preInitialize(GLShaderSource& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<GLSnippet> createGLSnippet(GraphicsContext& graphicsContext) const;

private:
    GLSnippetDelegate& _wrapper;
    sp<GLShader> _shader;
    GLBuffer _array_buffer;
};

CoreGLSnippet::CoreGLSnippet(GLSnippetDelegate& wrapper, const sp<GLShader>& shader, const GLBuffer& arrayBuffer)
    : _wrapper(wrapper), _shader(shader), _array_buffer(arrayBuffer)
{
}

void CoreGLSnippet::preInitialize(GLShaderSource& source)
{
    _wrapper.preInitialize(source);
}

void CoreGLSnippet::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context)
{
    const sp<GLSnippet> delegate = _wrapper._core;
    _wrapper._core = createGLSnippet(graphicsContext);
    _wrapper.preCompile(graphicsContext, context);
}

void CoreGLSnippet::preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context)
{
    const sp<GLSnippet> delegate = _wrapper._core;
    _wrapper._core = createGLSnippet(graphicsContext);
    _wrapper.preDraw(graphicsContext, shader, context);
}

void CoreGLSnippet::postDraw(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("You're not supposed to be here");
}

sp<GLSnippet> CoreGLSnippet::createGLSnippet(GraphicsContext& graphicsContext) const
{
    const sp<GLSnippet> coreGLSnippet = graphicsContext.glContext()->createCoreGLSnippet(graphicsContext.glResourceManager(), _shader, _array_buffer);
    return _shader->snippet() ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(coreGLSnippet, _shader->snippet())) : coreGLSnippet;
}

GLSnippetDelegate::GLSnippetDelegate(const sp<GLShader>& shader, const GLBuffer& arrayBuffer)
    : _core(sp<CoreGLSnippet>::make(*this, shader, arrayBuffer))
{
}

void GLSnippetDelegate::preInitialize(GLShaderSource& source)
{
    _core->preInitialize(source);
}

void GLSnippetDelegate::preCompile(GraphicsContext& graphicsContext, GLShaderPreprocessorContext& context)
{
    _core->preCompile(graphicsContext, context);
}

void GLSnippetDelegate::preDraw(GraphicsContext& graphicsContext, const GLShader& shader, const GLDrawingContext& context)
{
    _core->preDraw(graphicsContext, shader, context);
    if(_delegate)
        _delegate->preDraw(graphicsContext, shader, context);
}

void GLSnippetDelegate::postDraw(GraphicsContext& graphicsContext)
{
    _core->postDraw(graphicsContext);
    if(_delegate)
        _delegate->postDraw(graphicsContext);
}

void GLSnippetDelegate::link(const sp<GLSnippet>& snippet)
{
    _delegate = _delegate ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(_delegate, snippet)) : snippet;
}

}