#include "graphics/impl/renderer/renderer_with_position.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"

namespace ark {

RendererWithPosition::RendererWithPosition(const sp<Renderer>& renderer, const sp<Vec>& position)
    : _renderer(renderer), _position(position)
{
    DCHECK(renderer && position, "Arguments must not be null");
}

void RendererWithPosition::render(RenderRequest& renderRequest, float x, float y)
{
    const V position = _position->val();
    _renderer->render(renderRequest, x + position.x(), y + position.y());
}

RendererWithPosition::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _position(parent.ensureBuilder<Vec>(value))
{
}

sp<Renderer> RendererWithPosition::DECORATOR::build(const Scope& args)
{
    const sp<Renderer> bean = _delegate->build(args);
    return sp<Renderer>::adopt(new RendererWithPosition(bean, _position->build(args))).absorb(bean);
}

}
