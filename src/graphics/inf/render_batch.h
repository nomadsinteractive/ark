#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderBatch {
public:
    RenderBatch(sp<Boolean> disposed = nullptr);
    virtual ~RenderBatch() = default;

    const sp<Boolean>& disposed() const;

    virtual std::vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) = 0;

protected:
    sp<Boolean> _disposed;
};

}
