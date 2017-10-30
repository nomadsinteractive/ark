#ifndef ARK_CORE_IMPL_NUMERIC_LINEAR_H_
#define ARK_CORE_IMPL_NUMERIC_LINEAR_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Linear : public Numeric {
public:
    Linear(const sp<Numeric>& t, float v, float s);

    virtual float val() override;

//  [[plugin::builder("linear")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _t;
        sp<Builder<Numeric>> _v, _s;
    };

private:
    sp<Numeric> _t;
    float _v, _s;
};

}

#endif
