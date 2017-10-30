#include "graphics/impl/renderer/renderer_with_visibility.h"

#include "core/base/bean_factory.h"
#include "core/inf/variable.h"

namespace ark {

RendererWithVisibility::RendererWithVisibility(const sp<Renderer>& renderer, const sp<Boolean>& visibility)
    : _renderer(renderer), _visibility(visibility)
{
    NOT_NULL(_renderer);
    NOT_NULL(_visibility);
}

void RendererWithVisibility::render(RenderCommandPipeline& pipeline, float x, float y)
{
    if(_visibility->val())
        _renderer->render(pipeline, x, y);
}

RendererWithVisibility::DECORATOR::DECORATOR(BeanFactory& parent, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _visibility(parent.ensureBuilder<Boolean>(value))
{
}

sp<Renderer> RendererWithVisibility::DECORATOR::build(const sp<Scope>& args)
{
    const sp<Renderer> delegate = _delegate->build(args);
    return sp<Renderer>::adopt(new RendererWithVisibility(delegate, _visibility->build(args))).absorb(delegate);
}

}
