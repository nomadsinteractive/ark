#include "graphics/impl/renderer/renderer_style_visibility.h"

#include "core/base/bean_factory.h"
#include "core/traits/visibility.h"

#include "graphics/base/render_request.h"

namespace ark {

RendererStyleVisibility::RendererStyleVisibility(sp<Renderer> renderer, sp<Visibility> visibility)
    : _renderer(std::move(renderer)), _visibility(std::move(visibility))
{
    DASSERT(_renderer);
    DASSERT(_visibility);
}

void RendererStyleVisibility::render(RenderRequest& renderRequest, const V3& position)
{
    _visibility->update(renderRequest.timestamp());

    if(_visibility->val())
        _renderer->render(renderRequest, position);
}

RendererStyleVisibility::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _visibility(factory.ensureBuilder<Visibility>(value))
{
}

sp<Renderer> RendererStyleVisibility::STYLE::build(const Scope& args)
{
    return sp<RendererStyleVisibility>::make(_delegate->build(args), _visibility->build(args));
}

}
