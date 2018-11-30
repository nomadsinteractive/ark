#include "renderer/base/gl_snippet_delegate.h"

#include "renderer/base/gl_buffer.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/gl_snippet/gl_snippet_linked_chain.h"

namespace ark {

class CoreGLSnippet : public GLSnippet {
public:
    CoreGLSnippet(GLSnippetDelegate& wrapper, const sp<Shader>& shader);

    virtual void preInitialize(PipelineLayout& source) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<GLSnippet> createGLSnippet(GraphicsContext& graphicsContext, const DrawingContext& context) const;

private:
    GLSnippetDelegate& _wrapper;
    sp<Shader> _shader;
};

CoreGLSnippet::CoreGLSnippet(GLSnippetDelegate& wrapper, const sp<Shader>& shader)
    : _wrapper(wrapper), _shader(shader)
{
}

void CoreGLSnippet::preInitialize(PipelineLayout& source)
{
    _wrapper.preInitialize(source);
}

void CoreGLSnippet::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context)
{
    _wrapper.preCompile(graphicsContext, context);
}

void CoreGLSnippet::preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context)
{
    const sp<GLSnippet> delegate = _wrapper._core;
    _wrapper._core = createGLSnippet(graphicsContext, context);
    _wrapper.preDraw(graphicsContext, shader, context);
}

void CoreGLSnippet::postDraw(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("You're not supposed to be here");
}

sp<GLSnippet> CoreGLSnippet::createGLSnippet(GraphicsContext& graphicsContext, const DrawingContext& context) const
{
    const sp<GLSnippet> coreGLSnippet = graphicsContext.glContext()->createCoreGLSnippet(graphicsContext.resourceManager(), _shader, context._shader_bindings);
    return _shader->snippet() ? sp<GLSnippet>::adopt(new GLSnippetLinkedChain(coreGLSnippet, _shader->snippet())) : coreGLSnippet;
}

GLSnippetDelegate::GLSnippetDelegate(const sp<Shader>& shader)
    : _core(sp<CoreGLSnippet>::make(*this, shader))
{
}

void GLSnippetDelegate::preInitialize(PipelineLayout& source)
{
    _core->preInitialize(source);
}

void GLSnippetDelegate::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context)
{
    _core->preCompile(graphicsContext, context);
}

void GLSnippetDelegate::preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context)
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
