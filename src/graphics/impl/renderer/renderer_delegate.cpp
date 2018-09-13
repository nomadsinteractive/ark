#include "graphics/impl/renderer/renderer_delegate.h"

#include "core/base/bean_factory.h"

namespace ark {

RendererDelegate::RendererDelegate(const sp<Renderer>& delegate)
    : _delegate(delegate)
{
}

void RendererDelegate::render(RenderRequest& renderRequest, float x, float y)
{
    if(_delegate)
        _delegate->render(renderRequest, x, y);
}

void RendererDelegate::setRendererDelegate(const sp<Renderer>& delegate)
{
    _delegate = delegate;
}

const sp<Renderer>& RendererDelegate::delegate() const
{
    return _delegate;
}

}
