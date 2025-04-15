#include "graphics/util/renderer_type.h"

#include "core/base/enum.h"
#include "core/collection/traits.h"
#include "core/components/discarded.h"
#include "core/util/strings.h"
#include "core/util/string_convert.h"

#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_wrapper.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

sp<Renderer> RendererType::create(sp<Renderer> delegate)
{
    return wrap(std::move(delegate));
}

sp<Renderer> RendererType::create(Vector<sp<Renderer>> delegate)
{
    const sp<RenderGroup> rendererGroup = sp<RenderGroup>::make();
    for(sp<Renderer>& i : delegate)
        rendererGroup->addRenderer(std::move(i), {});
    return wrap(rendererGroup);
}

void RendererType::addRenderer(const sp<Renderer>& self, sp<Renderer> renderer, const Traits& traits)
{
    const sp<Renderer::Group> rendererGroup = self.ensureInstance<Renderer::Group>("Cannot call addRenderer on a none-group renderer");
    rendererGroup->addRenderer(std::move(renderer), traits);
}

sp<Renderer> RendererType::wrap(sp<Renderer> self)
{
    return sp<Renderer>::make<RendererWrapper>(std::move(self));
}

sp<Renderer> RendererType::reset(const sp<Renderer>& self, sp<Renderer> wrapped)
{
    const sp<Wrapper<Renderer>> rd = self.ensureInstance<Wrapper<Renderer>>("Renderer is not an instance of Wrapper<Renderer>");
    return rd->reset(std::move(wrapped));
}

template<> ARK_API RendererType::Priority StringConvert::eval<RendererType::Priority>(const String& expr)
{
    if(Strings::isNumeric(expr))
        return static_cast<RendererType::Priority>(Strings::eval<uint32_t>(expr));

    constexpr enums::LookupTable<RendererType::Priority, RendererType::PRIORITY_COUNT> priorities = {{
        {"ui", RendererType::PRIORITY_UI},
        {"default", RendererType::PRIORITY_DEFAULT},
        {"ui_blend", RendererType::PRIORITY_UI_BLEND},
        {"ui_text", RendererType::PRIORITY_UI_TEXT},
        {"render_layer", RendererType::PRIORITY_RENDER_LAYER},
        {"control", RendererType::PRIORITY_CONTROL},
    }};
    return enums::lookup(priorities, expr);
}

}
