#include "graphics/impl/renderer/renderer_delegate.h"

#include "core/base/bean_factory.h"

namespace ark {

RendererDelegate::RendererDelegate()
{
}

RendererDelegate::RendererDelegate(const sp<Renderer>& delegate)
    : _delegate(delegate)
{
}

void RendererDelegate::render(RenderCommandPipeline& pipeline, float x, float y)
{
    if(_delegate)
        _delegate->render(pipeline, x, y);
}

void RendererDelegate::setRendererDelegate(const sp<Renderer>& delegate)
{
    _delegate = delegate;
}

const sp<Renderer>& RendererDelegate::delegate() const
{
    return _delegate;
}

RendererDelegate::BUILDER::BUILDER(BeanFactory& beanFactory, const document& manifest)
    : _delegate(beanFactory.getBuilder<Renderer>(manifest, Constants::Attributes::DELEGATE))
{
}

sp<Renderer> RendererDelegate::BUILDER::build(const sp<Scope>& args)
{
    return sp<RendererDelegate>::make(_delegate->build(args));
}

}
