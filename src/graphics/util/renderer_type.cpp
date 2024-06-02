#include "graphics/util/renderer_type.h"

#include "core/collection/traits.h"
#include "core/traits/expendable.h"
#include "core/traits/visibility.h"
#include "core/types/safe_ptr.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"

#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_style_position.h"
#include "graphics/impl/renderer/renderer_wrapper.h"

namespace ark {

sp<Renderer> RendererType::create(const sp<Renderer>& delegate)
{
    return wrap(delegate);
}

void RendererType::addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer, Traits traits)
{
    DCHECK(self.isInstance<Renderer::Group>(), "Cannot call addRenderer on a none-group renderer");
    const sp<Renderer::Group> rendererGroup = self.tryCast<Renderer::Group>();
    rendererGroup->addRenderer(renderer, std::move(traits));
}

sp<Renderer> RendererType::wrap(const sp<Renderer>& self)
{
    return sp<RendererWrapper>::make(self);
}

sp<Renderer> RendererType::makeDisposable(const sp<Renderer>& self, const sp<Boolean>& disposed)
{
    WARN("makeDisposable is deprecated");
    return self;
}

sp<Renderer> RendererType::makeVisible(const sp<Renderer>& self, const sp<Boolean>& visibility)
{
    WARN("makeVisible is deprecated");
    return self;
}

sp<Renderer> RendererType::makeAutoRelease(const sp<Renderer>& self, int32_t refCount)
{
    WARN("makeAutoRelease is deprecated");
    return self;
}

SafePtr<Size> RendererType::size(const sp<Renderer>& self)
{
    return SafePtr<Size>();
}

sp<Renderer> RendererType::wrapped(const sp<Renderer>& self)
{
    const sp<Wrapper<Renderer>> rd = self.tryCast<Wrapper<Renderer>>();
    CHECK_WARN(rd, "Renderer is not an instance of Wrapper<Renderer>");
    return rd ? rd->wrapped() : nullptr;
}

void RendererType::setWrapped(const sp<Renderer>& self, sp<Renderer> wrapped)
{
    const sp<Wrapper<Renderer>> rd = self.tryCast<Wrapper<Renderer>>();
    CHECK(rd, "Renderer is not an instance of Wrapper<Renderer>");
    rd->reset(std::move(wrapped));
}

sp<Renderer> ark::RendererType::translate(const sp<Renderer>& self, const sp<Vec3>& position)
{
    return sp<RendererStylePosition>::make(self, position);
}

}
