#pragma once

#include "core/forwarding.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanByTimeout final : public Boolean {
public:
    BooleanByTimeout(sp<Numeric> duration, float timeout);

    bool val() override;
    bool update(uint64_t timestamp) override;

private:
    sp<Numeric> _duration;
    float _timeout;
};

}
