#include "graphics/impl/renderer/renderer_with_position.h"

#include "core/inf/variable.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"

namespace ark {

RendererWithPosition::RendererWithPosition(const sp<Renderer>& renderer, const sp<VV>& position)
    : _renderer(renderer), _position(position)
{
    DCHECK(renderer && position, "Arguments must be not null");
}

void RendererWithPosition::render(RenderCommandPipeline& pipeline, float x, float y)
{
    const V position = _position->val();
    _renderer->render(pipeline, x + position.x(), y + position.y());
}

RendererWithPosition::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _position(parent.ensureBuilder<VV>(value))
{
}

sp<Renderer> RendererWithPosition::DECORATOR::build(const sp<Scope>& args)
{
    const sp<Renderer> bean = _delegate->build(args);
    return sp<Renderer>::adopt(new RendererWithPosition(bean, _position->build(args))).absorb(bean);
}

}
