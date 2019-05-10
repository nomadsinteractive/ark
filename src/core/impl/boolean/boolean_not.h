#ifndef ARK_CORE_IMPL_BOOLEAN_BOOLEAN_NOT_H_
#define ARK_CORE_IMPL_BOOLEAN_BOOLEAN_NOT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanNot : public Boolean {
public:
    BooleanNot(const sp<Boolean>& delegate);

    virtual bool val() override;

private:
    sp<Boolean> _delegate;

};

}

#endif
