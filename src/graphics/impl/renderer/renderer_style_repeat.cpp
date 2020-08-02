#include "graphics/impl/renderer/renderer_style_repeat.h"

#include "core/base/bean_factory.h"
#include "core/epi/visibility.h"

#include "graphics/base/render_request.h"


namespace ark {

RendererStyleRepeat::RendererStyleRepeat(sp<Renderer> delegate, sp<Integer> repeat)
    : _delegate(std::move(delegate)), _repeat(std::move(repeat))
{
    DASSERT(_delegate);
}

void RendererStyleRepeat::render(RenderRequest& renderRequest, const V3& position)
{
    _repeat->update(renderRequest.timestamp());

    int32_t repeat = _repeat->val();
    for(int32_t i = 0; i < repeat; ++i)
        _delegate->render(renderRequest, position);
}

RendererStyleRepeat::STYLE::STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& value)
    : _delegate(delegate), _repeat(factory.ensureBuilder<Integer>(value))
{
}

sp<Renderer> RendererStyleRepeat::STYLE::build(const Scope& args)
{
    const sp<Renderer> delegate = _delegate->build(args);
    return sp<RendererStyleRepeat>::make(delegate, _repeat->build(args)).absorb(delegate);
}

}
