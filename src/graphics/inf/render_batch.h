#pragma once

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class ARK_API RenderBatch {
public:
    RenderBatch(sp<Boolean> discarded = nullptr);
    virtual ~RenderBatch() = default;

    const sp<Boolean>& discarded() const;
    void setDiscarded(sp<Boolean> discarded);

    virtual Vector<sp<LayerContext>>& snapshot(const RenderRequest& renderRequest) = 0;

protected:
    sp<Boolean> _discarded;
};

}
