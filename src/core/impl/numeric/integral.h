#ifndef ARK_CORE_IMPL_NUMERIC_INTEGRAL_H_
#define ARK_CORE_IMPL_NUMERIC_INTEGRAL_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Integral : public Numeric {
public:
    Integral(float s, const sp<Numeric>& t, const sp<Numeric>& velocity);

    virtual float val() override;

//  [[plugin::builder("integral")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _s;
        sp<Builder<Numeric>> _t;
        sp<Builder<Numeric>> _v;
    };

private:
    float _s;
    float _last_v;
    float _last_t;
    sp<Numeric> _t;
    sp<Numeric> _v;

};

}

#endif
