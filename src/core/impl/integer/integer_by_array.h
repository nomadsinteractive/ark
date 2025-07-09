#pragma once

#include "core/forwarding.h"
#include "core/base/wrapper.h"
#include "core/inf/variable.h"
#include "core/components/with_callback.h"
#include "core/util/integer_type.h"

namespace ark {

class IntegerByArray final : public Wrapper<IntArray>, public WithCallback, public Integer {
public:
    IntegerByArray(sp<IntArray> array, IntegerType::Repeat repeat, sp<Runnable> callback);

    int32_t val() override;
    bool update(uint64_t timestamp) override;

private:
    IntegerType::Repeat _repeat;
    int32_t _position;
    int32_t _step;
};

}
