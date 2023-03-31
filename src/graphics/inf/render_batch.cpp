#include "graphics/inf/render_batch.h"

#include "core/inf/variable.h"

namespace ark {

RenderBatch::RenderBatch(sp<Boolean> disposed)
    : _disposed(std::move(disposed))
{
}

const sp<Boolean>& RenderBatch::disposed() const
{
    return _disposed;
}

}
