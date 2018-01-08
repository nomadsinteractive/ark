#ifndef ARK_CORE_IMPL_NUMERIC_NUMERIC_WITH_BOUNDARY_H_
#define ARK_CORE_IMPL_NUMERIC_NUMERIC_WITH_BOUNDARY_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class NumericWithBoundary : public Numeric {
public:
    NumericWithBoundary(const sp<Numeric>& delegate, const sp<Numeric>& boundary, const sp<Runnable>& oncross);

    virtual float val() override;

//  [[plugin::builder("boundary")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _boundary;
        sp<Builder<Runnable>> _oncross;

    };

private:
    sp<Numeric> _delegate;
    sp<Numeric> _boundary;

    sp<Runnable> _oncross;

    bool _is_greater;
};

}

#endif
