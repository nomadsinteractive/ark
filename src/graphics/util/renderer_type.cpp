#include "graphics/util/renderer_type.h"

#include "core/epi/disposed.h"
#include "core/epi/visibility.h"
#include "core/types/safe_ptr.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_style_position.h"
#include "graphics/impl/renderer/renderer_style_visibility.h"
#include "graphics/impl/renderer/renderer_wrapper.h"

namespace ark {

sp<Renderer> RendererType::create(const sp<Renderer>& delegate)
{
    return wrap(delegate);
}

void RendererType::addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer)
{
    DCHECK(self.template is<Renderer::Group>(), "Cannot call addRenderer on a none-group renderer");
    const sp<Renderer::Group> rendererGroup = self.template as<Renderer::Group>();
    rendererGroup->addRenderer(renderer);
}

sp<Renderer> RendererType::wrap(const sp<Renderer>& self)
{
    return sp<RendererWrapper>::make(self);
}

sp<Renderer> RendererType::makeDisposable(const sp<Renderer>& self, const sp<Boolean>& disposed)
{
    return self.absorb(disposed ? sp<Disposed>::make(disposed) : sp<Disposed>::make());
}

sp<Renderer> RendererType::makeVisible(const sp<Renderer>& self, const sp<Boolean>& visibility)
{
    return self.absorb(sp<Visibility>::make(visibility));
}

sp<Renderer> RendererType::makeAutoRelease(const sp<Renderer>& self, int32_t refCount)
{
    return makeDisposable(self, sp<BooleanByWeakRef<Renderer>>::make(self, refCount));
}

SafePtr<Size> RendererType::size(const sp<Renderer>& self)
{
    if(self.template is<Block>())
    {
        sp<Block> block = self.template as<Block>();
        if(block)
            return block->size();
    }
    return SafePtr<Size>();
}

const sp<Renderer>& RendererType::wrapped(const sp<Renderer>& self)
{
    const sp<Wrapper<Renderer>> rd = self.as<Wrapper<Renderer>>();
    WARN(rd, "Renderer is not an instance of Wrapper<Renderer>");
    return rd ? rd->wrapped() : sp<Renderer>::null();
}

void RendererType::setWrapped(const sp<Renderer>& self, sp<Renderer> wrapped)
{
    const sp<Wrapper<Renderer>> rd = self.as<Wrapper<Renderer>>();
    CHECK(rd, "Renderer is not an instance of Wrapper<Renderer>");
    rd->reset(std::move(wrapped));
}

sp<Renderer> ark::RendererType::translate(const sp<Renderer>& self, const sp<Vec3>& position)
{
    return sp<RendererStylePosition>::make(self, position);
}

}
