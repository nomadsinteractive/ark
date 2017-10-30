#ifndef ARK_GRAPHICS_IMPL_VV2_VV2_WITH_TRANSFORM_H_
#define ARK_GRAPHICS_IMPL_VV2_VV2_WITH_TRANSFORM_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class VV2WithTransform : public VV2 {
public:
    VV2WithTransform(const sp<VV2>& delegate, const sp<VV2>& org, const sp<Transform>& transform);

    virtual V2 val() override;

private:
    const sp<VV2> _delegate;
    const sp<VV2> _org;
    const sp<Transform> _transform;
};

}

#endif
