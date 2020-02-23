#include "graphics/util/renderer_util.h"

#include "core/epi/disposed.h"
#include "core/types/safe_ptr.h"
#include "core/impl/boolean/boolean_by_weak_ref.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_with_position.h"
#include "graphics/impl/renderer/renderer_wrapper.h"

namespace ark {

sp<Renderer> RendererUtil::create(const sp<Renderer>& delegate)
{
    return wrap(delegate);
}

void RendererUtil::addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer)
{
    if(self.template is<Renderer::Group>())
    {
        const sp<Renderer::Group> rendererGroup = self.template as<Renderer::Group>();
        rendererGroup->addRenderer(renderer);
    }
}

sp<Renderer> RendererUtil::wrap(const sp<Renderer>& self)
{
    return sp<RendererWrapper>::make(self);
}

void RendererUtil::dispose(const sp<Renderer>& self)
{
    if(self.template is<Disposed>())
    {
        const sp<Disposed> m = self.template as<Disposed>();
        if(m)
            m->dispose();
    }
}

sp<Renderer> RendererUtil::makeDisposable(const sp<Renderer>& self, const sp<Boolean>& disposed)
{
    return self.absorb(disposed ? sp<Disposed>::make(disposed) : sp<Disposed>::make());
}

sp<Renderer> RendererUtil::makeAutoRelease(const sp<Renderer>& self, int32_t refCount)
{
    return makeDisposable(self, sp<BooleanByWeakRef<Renderer>>::make(self, refCount));
}

SafePtr<Size> RendererUtil::size(const sp<Renderer>& self)
{
    if(self.template is<Block>())
    {
        sp<Block> block = self.template as<Block>();
        if(block)
            return block->size();
    }
    return SafePtr<Size>();
}

const sp<Renderer>& RendererUtil::delegate(const sp<Renderer>& self)
{
    const sp<Delegate<Renderer>> rd = self.as<Delegate<Renderer>>();
    DWARN(rd, "Renderer is not an instance of Delegate<Renderer>");
    return rd ? rd->delegate() : sp<Renderer>::null();
}

void RendererUtil::setDelegate(const sp<Renderer>& self, const sp<Renderer>& delegate)
{
    const sp<Delegate<Renderer>> rd = self.as<Delegate<Renderer>>();
    DCHECK(rd, "Renderer is not an instance of Delegate<Renderer>");
    rd->setDelegate(delegate);
}

sp<Renderer> ark::RendererUtil::translate(const sp<Renderer>& self, const sp<Vec3>& position)
{
    return sp<RendererWithPosition>::make(self, position);
}

}
