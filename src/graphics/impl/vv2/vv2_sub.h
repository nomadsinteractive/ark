#ifndef ARK_GRAPHICS_IMPL_VV2_VV2_SUB_H_
#define ARK_GRAPHICS_IMPL_VV2_VV2_SUB_H_

#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class VV2Sub : public VV2 {
public:
    VV2Sub(const sp<VV2>& lvalue, const sp<VV2>& rvalue);

    virtual V2 val() override;

private:
    sp<VV2> _lvalue;
    sp<VV2> _rvalue;
};

}

#endif
