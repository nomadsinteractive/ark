#ifndef ARK_CORE_IMPL_NUMERIC_ABS_H_
#define ARK_CORE_IMPL_NUMERIC_ABS_H_

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Abs : public Numeric {
public:
    Abs(const sp<Numeric>& x);

    virtual float val() override;

//  [[plugin::function("abs")]]
    static sp<Numeric> call(const sp<Numeric>& x);

private:
    sp<Numeric> _x;

};

}

#endif
