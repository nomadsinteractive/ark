#include "graphics/inf/render_batch.h"

namespace ark {

RenderBatch::RenderBatch(sp<Boolean> discarded)
    : _discarded(std::move(discarded))
{
}

const sp<Boolean>& RenderBatch::discarded() const
{
    return _discarded;
}

void RenderBatch::setDiscarded(sp<Boolean> discarded)
{
    _discarded = std::move(discarded);
}
}
