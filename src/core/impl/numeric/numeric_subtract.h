#ifndef ARK_CORE_IMPL_NUMERIC_NUMERIC_SUBTRACT_H_
#define ARK_CORE_IMPL_NUMERIC_NUMERIC_SUBTRACT_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class NumericSubtract : public Numeric {
public:
    NumericSubtract(const sp<Numeric>& a1, const sp<Numeric>& a2);

    virtual float val() override;

//  [[plugin::style("subtract"]]
    class STYLE : public Builder<Numeric> {
    public:
        STYLE(BeanFactory& factory, const sp<Builder<Numeric>>& delegate, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _subtract;
    };


private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

}

#endif
