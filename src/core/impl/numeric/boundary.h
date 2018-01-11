#ifndef ARK_CORE_IMPL_NUMERIC_BOUNDARY_H_
#define ARK_CORE_IMPL_NUMERIC_BOUNDARY_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Boundary : public Numeric {
public:
    Boundary(const sp<Numeric>& delegate, const sp<Expectation>& expectation);

    virtual float val() override;

//  [[plugin::builder("boundary")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Expectation>> _expectation;

    };

private:
    sp<Numeric> _delegate;
    sp<Expectation> _expectation;

    bool _is_greater;
};

}

#endif
