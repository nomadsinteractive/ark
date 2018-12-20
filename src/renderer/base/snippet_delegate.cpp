#include "renderer/base/snippet_delegate.h"

#include "renderer/base/buffer.h"
#include "renderer/base/gl_context.h"
#include "renderer/base/drawing_context.h"
#include "renderer/base/shader.h"
#include "renderer/base/graphics_context.h"
#include "renderer/impl/snippet/snippet_linked_chain.h"

namespace ark {

class CoreGLSnippet : public Snippet {
public:
    CoreGLSnippet(SnippetDelegate& wrapper, const sp<Shader>& shader);

    virtual void preInitialize(PipelineBuildingContext& context) override;
    virtual void preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context) override;
    virtual void preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context) override;
    virtual void postDraw(GraphicsContext& graphicsContext) override;

private:
    sp<Snippet> createGLSnippet(GraphicsContext& graphicsContext, const DrawingContext& context) const;

private:
    SnippetDelegate& _wrapper;
    sp<Shader> _shader;
};

CoreGLSnippet::CoreGLSnippet(SnippetDelegate& wrapper, const sp<Shader>& shader)
    : _wrapper(wrapper), _shader(shader)
{
}

void CoreGLSnippet::preInitialize(PipelineBuildingContext& context)
{
    _wrapper.preInitialize(context);
}

void CoreGLSnippet::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context)
{
    _wrapper.preCompile(graphicsContext, context);
}

void CoreGLSnippet::preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context)
{
    const sp<Snippet> delegate = _wrapper._core;
    _wrapper._core = createGLSnippet(graphicsContext, context);
    _wrapper.preDraw(graphicsContext, shader, context);
}

void CoreGLSnippet::postDraw(GraphicsContext& /*graphicsContext*/)
{
    DFATAL("You're not supposed to be here");
}

sp<Snippet> CoreGLSnippet::createGLSnippet(GraphicsContext& graphicsContext, const DrawingContext& context) const
{
    const sp<Snippet> coreGLSnippet = graphicsContext.glContext()->createCoreGLSnippet(graphicsContext.resourceManager(), _shader, context._shader_bindings);
    return _shader->snippet() ? sp<Snippet>::adopt(new SnippetLinkedChain(coreGLSnippet, _shader->snippet())) : coreGLSnippet;
}

SnippetDelegate::SnippetDelegate(const sp<Shader>& shader)
    : _core(sp<CoreGLSnippet>::make(*this, shader))
{
}

void SnippetDelegate::preInitialize(PipelineBuildingContext& context)
{
    _core->preInitialize(context);
}

void SnippetDelegate::preCompile(GraphicsContext& graphicsContext, PipelineBuildingContext& context)
{
    _core->preCompile(graphicsContext, context);
}

void SnippetDelegate::preDraw(GraphicsContext& graphicsContext, const Shader& shader, const DrawingContext& context)
{
    _core->preDraw(graphicsContext, shader, context);
    if(_delegate)
        _delegate->preDraw(graphicsContext, shader, context);
}

void SnippetDelegate::postDraw(GraphicsContext& graphicsContext)
{
    _core->postDraw(graphicsContext);
    if(_delegate)
        _delegate->postDraw(graphicsContext);
}

void SnippetDelegate::link(const sp<Snippet>& snippet)
{
    _delegate = _delegate ? sp<Snippet>::adopt(new SnippetLinkedChain(_delegate, snippet)) : snippet;
}

}
