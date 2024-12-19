#include "graphics/util/renderer_type.h"

#include "core/collection/traits.h"
#include "core/traits/expendable.h"
#include "core/traits/visibility.h"
#include "graphics/impl/renderer/renderer_group.h"

#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_style_position.h"
#include "graphics/impl/renderer/renderer_wrapper.h"
#include "graphics/impl/renderer/render_group.h"

namespace ark {

sp<Renderer> RendererType::create(const sp<Renderer>& delegate)
{
    return wrap(delegate);
}

sp<Renderer> RendererType::create(const std::vector<sp<Renderer>>& renderers)
{
    const sp<RendererPhrase> rendererGroup = sp<RendererPhrase>::make();
    for(const sp<Renderer>& i : renderers)
        rendererGroup->addRenderer(i, {});
    return wrap(rendererGroup);
}

void RendererType::addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer, const Traits& traits)
{
    DCHECK(self.isInstance<Renderer::Group>(), "Cannot call addRenderer on a none-group renderer");
    const sp<Renderer::Group> rendererGroup = self.tryCast<Renderer::Group>();
    rendererGroup->addRenderer(renderer, traits);
}

sp<Renderer> RendererType::wrap(const sp<Renderer>& self)
{
    return sp<Renderer>::make<RendererWrapper>(self);
}

sp<Renderer> RendererType::reset(const sp<Renderer>& self, sp<Renderer> wrapped)
{
    const sp<Wrapper<Renderer>> rd = self.tryCast<Wrapper<Renderer>>();
    CHECK(rd, "Renderer is not an instance of Wrapper<Renderer>");
    return rd->reset(std::move(wrapped));
}

sp<Renderer> RendererType::translate(const sp<Renderer>& self, const sp<Vec3>& position)
{
    return sp<RendererStylePosition>::make(self, position);
}

}
