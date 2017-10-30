#ifndef ARK_CORE_IMPL_NUMERIC_MIN_H_
#define ARK_CORE_IMPL_NUMERIC_MIN_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Min : public Numeric {
public:
    Min(const sp<Numeric>& delegate, const sp<Numeric>& min);

    virtual float val() override;

//  [[plugin::function("min")]]
    static sp<Numeric> min(const sp<Numeric>& a, const sp<Numeric>& b);

//  [[plugin::style("min")]]
    class DECORATOR : public Builder<Numeric> {
    public:
        DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _min;
    };

private:
    sp<Numeric> _delegate;
    sp<Numeric> _min;
};

}

#endif
