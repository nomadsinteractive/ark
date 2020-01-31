#include "graphics/impl/renderer/renderer_with_visibility.h"

#include "core/base/bean_factory.h"
#include "core/epi/visibility.h"

#include "graphics/base/render_request.h"


namespace ark {

RendererWithVisibility::RendererWithVisibility(const sp<Renderer>& renderer, const sp<Visibility>& visibility)
    : _renderer(renderer), _visibility(visibility)
{
    DASSERT(_renderer);
    DASSERT(_visibility);
}

void RendererWithVisibility::render(RenderRequest& renderRequest, const V3& position)
{
    _visibility->update(renderRequest.timestamp());

    if(_visibility->val())
        _renderer->render(renderRequest, position);
}

RendererWithVisibility::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _visibility(factory.ensureBuilder<Visibility>(value))
{
}

sp<Renderer> RendererWithVisibility::STYLE::build(const Scope& args)
{
    const sp<Renderer> delegate = _delegate->build(args);
    const sp<Visibility> visibility =  _visibility->build(args);
    return sp<RendererWithVisibility>::make(delegate, visibility).absorb(visibility).absorb(delegate);
}

}
