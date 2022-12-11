#include "graphics/impl/renderer/renderer_style_bounds.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/inf/block.h"
#include "graphics/base/v3.h"
#include "graphics/base/render_request.h"
#include "graphics/base/size.h"
#include "graphics/impl/vec/vec4_impl.h"

namespace ark {

RendererStyleBounds::RendererStyleBounds(const sp<Renderer>& renderer, const sp<Vec4>& bounds)
    : _renderer(renderer), _bounds(bounds.as<Vec4Impl>()), _size(_renderer.as<Block>()->size())
{
    DCHECK(_renderer && _bounds, "Arguments must not be null");
}

void RendererStyleBounds::render(RenderRequest& renderRequest, const V3& position)
{
    _renderer->render(renderRequest, position);
    _bounds->set(V4(position.x(), position.y(), position.x() + _size->widthAsFloat(), position.y() + _size->heightAsFloat()));
}

RendererStyleBounds::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _bounds(factory.ensureBuilder<Vec4>(value))
{
}

sp<Renderer> RendererStyleBounds::STYLE::build(const Scope& args)
{
    const sp<Renderer> bean = _delegate->build(args);
    return sp<Renderer>::make<RendererStyleBounds>(bean, _bounds->build(args)).absorb(bean);
}

}
