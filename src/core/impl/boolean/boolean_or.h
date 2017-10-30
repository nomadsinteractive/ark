#ifndef ARK_CORE_IMPL_BOOLEAN_BOOLEAN_OR_H_
#define ARK_CORE_IMPL_BOOLEAN_BOOLEAN_OR_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanOr : public Boolean {
public:
    BooleanOr(const sp<Boolean>& a1, const sp<Boolean>& a2);

    virtual bool val() override;

private:
    sp<Boolean> _a1;
    sp<Boolean> _a2;

};

}

#endif
