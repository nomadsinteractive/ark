#ifndef ARK_CORE_IMPL_NUMERIC_UPPER_H_
#define ARK_CORE_IMPL_NUMERIC_UPPER_H_

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Upper : public Numeric {
public:
    Upper(const sp<Numeric>& delegate, const sp<Expectation>& expectation);

    virtual float val() override;

//  [[plugin::builder("upper")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Expectation>> _expectation;
    };

private:
    sp<Numeric> _delegate;
    sp<Expectation> _expectation;
};

}

#endif
