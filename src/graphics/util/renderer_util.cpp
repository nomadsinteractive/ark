#include "graphics/util/renderer_util.h"

#include "core/epi/disposed.h"
#include "core/types/safe_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderer/renderer_with_position.h"

namespace ark {

void RendererUtil::addRenderer(const sp<Renderer>& self, const sp<Renderer>& renderer)
{
    if(self.template is<Renderer::Group>())
    {
        const sp<Renderer::Group> rendererGroup = self.template as<Renderer::Group>();
        rendererGroup->addRenderer(renderer);
    }
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

sp<Renderer> ark::RendererUtil::translate(const sp<Renderer>& self, const sp<Vec2>& position)
{
    return sp<RendererWithPosition>::make(self, position);
}

}
