#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_ARRAY_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_ARRAY_H_

#include "core/forwarding.h"
#include "core/base/delegate.h"
#include "core/base/notifier.h"
#include "core/inf/variable.h"
#include "core/util/integer_type.h"

namespace ark {

class IntegerByArray : public Delegate<IntArray>, public Integer {
public:
    IntegerByArray(const sp<IntArray>& array, IntegerType::Repeat repeat, Notifier notifier);

    virtual int32_t val() override;
    virtual bool update(uint64_t timestamp) override;

private:
    IntegerType::Repeat _repeat;
    int32_t _position;
    int32_t _step;
    Notifier _notifier;
};

}

#endif
