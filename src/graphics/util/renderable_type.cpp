#include "graphics/util/renderable_type.h"

#include "graphics/impl/renderable/renderable_with_disposable.h"
#include "graphics/impl/renderable/renderable_with_updatable.h"

namespace ark {

sp<Renderable> RenderableType::create(sp<Renderable> renderable, sp<Updatable> updatable, sp<Boolean> discarded)
{
    ASSERT(renderable);
    if(updatable)
        renderable = sp<Renderable>::make<RenderableWithUpdatable>(std::move(renderable), std::move(updatable));
    if(discarded)
        renderable = sp<Renderable>::make<RenderableWithDiscarded>(std::move(renderable), std::move(discarded));
    return renderable;
}

}
