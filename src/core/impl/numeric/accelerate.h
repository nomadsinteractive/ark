#ifndef ARK_CORE_IMPL_NUMERIC_ACCELERATION_H_
#define ARK_CORE_IMPL_NUMERIC_ACCELERATION_H_

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Accelerate : public Numeric {
public:
    Accelerate(const sp<Numeric>& t, float v, float a, float s);

    virtual float val() override;

//  [[plugin::builder("accelerate")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _t;
        sp<Builder<Numeric>> _v, _a, _s;
    };

private:
    sp<Numeric> _t;
    float _v, _a, _s;
};

}

#endif
