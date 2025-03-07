#include "graphics/util/renderer_type.h"

#include "core/collection/traits.h"
#include "core/components/discarded.h"
#include "core/util/strings.h"
#include "core/util/string_convert.h"

#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_wrapper.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

sp<Renderer> RendererType::create(const sp<Renderer>& other)
{
    return wrap(other);
}

sp<Renderer> RendererType::create(const std::vector<sp<Renderer>>& other)
{
    const sp<RenderGroup> rendererGroup = sp<RenderGroup>::make();
    for(const sp<Renderer>& i : other)
        rendererGroup->addRenderer(i, {});
    return wrap(rendererGroup);
}

void RendererType::addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer, const Traits& traits)
{
    const sp<Renderer::Group> rendererGroup = self.ensureInstance<Renderer::Group>("Cannot call addRenderer on a none-group renderer");
    rendererGroup->addRenderer(renderer, traits);
}

sp<Renderer> RendererType::wrap(const sp<Renderer>& self)
{
    return sp<Renderer>::make<RendererWrapper>(self);
}

sp<Renderer> RendererType::reset(const sp<Renderer>& self, sp<Renderer> wrapped)
{
    const sp<Wrapper<Renderer>> rd = self.ensureInstance<Wrapper<Renderer>>("Renderer is not an instance of Wrapper<Renderer>");
    return rd->reset(std::move(wrapped));
}

template<> ARK_API RendererType::Priority StringConvert::eval<RendererType::Priority>(const String& expr)
{
    if(Strings::isNumeric(expr))
        return static_cast<RendererType::Priority>(Strings::eval<int32_t>(expr));
    if(expr.startsWith("ui_text"))
        return RendererType::PRIORITY_UI_TEXT;
    if(expr.startsWith("ui_blend"))
        return RendererType::PRIORITY_UI_BLEND;
    if(expr.startsWith("ui"))
        return RendererType::PRIORITY_UI;
    return RendererType::PRIORITY_DEFAULT;
}

}
