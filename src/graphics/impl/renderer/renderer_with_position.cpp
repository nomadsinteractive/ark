#include "graphics/impl/renderer/renderer_with_position.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v3.h"

namespace ark {

RendererWithPosition::RendererWithPosition(const sp<Renderer>& renderer, const sp<Vec3>& position)
    : _renderer(renderer), _position(position)
{
    DCHECK(renderer && position, "Arguments must not be null");
}

void RendererWithPosition::render(RenderRequest& renderRequest, const V3& position)
{
    _renderer->render(renderRequest, position + _position->val());
}

RendererWithPosition::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _position(factory.ensureBuilder<Vec3>(value))
{
}

sp<Renderer> RendererWithPosition::STYLE::build(const Scope& args)
{
    const sp<Renderer> bean = _delegate->build(args);
    return sp<Renderer>::adopt(new RendererWithPosition(bean, _position->build(args))).absorb(bean);
}

}
