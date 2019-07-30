#ifndef ARK_CORE_IMPL_INTEGER_INTEGER_REPEATER_H_
#define ARK_CORE_IMPL_INTEGER_INTEGER_REPEATER_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/integer_util.h"

namespace ark {

class IntegerByArray : public Integer {
public:
    struct Stub {
        Stub(const sp<IntArray>& array, IntegerUtil::Repeat repeat);

        int32_t val();

        sp<IntArray> _array;
        int32_t _position;
        IntegerUtil::Repeat _repeat;
        int32_t _step;
    };

    IntegerByArray(const sp<IntArray>& array, IntegerUtil::Repeat repeat);

    virtual int32_t val() override;

private:
    sp<Stub> _stub;
};

}

#endif
