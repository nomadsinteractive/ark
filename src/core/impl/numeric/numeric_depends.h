#ifndef ARK_CORE_IMPL_NUMERIC_NUMERIC_DEPENDS_H_
#define ARK_CORE_IMPL_NUMERIC_NUMERIC_DEPENDS_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class NumericDepends : public Numeric {
public:
    NumericDepends(const sp<Numeric>& delegate, const sp<Numeric>& depends);

    virtual float val() override;

//  [[plugin::style("depends")]]
    class DECORATOR : public Builder<Numeric> {
    public:
        DECORATOR(BeanFactory& beanFactory, const sp<Builder<Numeric>>& delegate, const String& style);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _depends;
    };

private:
    sp<Numeric> _delegate;
    sp<Numeric> _depends;
    float _val_delegate;
    float _val_depends;
};

}

#endif
