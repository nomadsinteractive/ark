#ifndef ARK_CORE_IMPL_NUMERIC_MULTIPLY_H_
#define ARK_CORE_IMPL_NUMERIC_MULTIPLY_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Multiply : public Numeric {
public:
    Multiply(const sp<Numeric>& a1, const sp<Numeric>& a2);

    virtual float val() override;

//  [[plugin::style("multiply")]]
    class DECORATOR : public Builder<Numeric> {
    public:
        DECORATOR(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& style);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _multiply;
    };

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

}

#endif
