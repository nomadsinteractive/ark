#include "graphics/impl/renderer/renderer_style_position.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v3.h"
#include "graphics/base/render_request.h"

namespace ark {

RendererStylePosition::RendererStylePosition(const sp<Renderer>& renderer, const sp<Vec3>& position)
    : _renderer(renderer), _position(position)
{
    DCHECK(_renderer && _position, "Arguments must not be null");
}

void RendererStylePosition::render(RenderRequest& renderRequest, const V3& position)
{
    _position->update(renderRequest.timestamp());
    _renderer->render(renderRequest, position + _position->val());
}

RendererStylePosition::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _position(factory.ensureBuilder<Vec3>(value))
{
}

sp<Renderer> RendererStylePosition::STYLE::build(const Scope& args)
{
    const sp<Renderer> bean = _delegate->build(args);
    return sp<Renderer>::make<RendererStylePosition>(bean, _position->build(args)).absorb(bean);
}

}
