#ifndef ARK_CORE_IMPL_BOOLEAN_BY_TIMEOUT_H_
#define ARK_CORE_IMPL_BOOLEAN_BY_TIMEOUT_H_

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanByTimeout : public Boolean {
public:
    BooleanByTimeout(sp<Numeric> duration, float timeout, bool timeoutValue);

    virtual bool val() override;
    virtual bool update(uint64_t timestamp) override;

private:
    sp<Numeric> _duration;
    float _timeout;
    bool _timeout_value;
};

}

#endif
