#ifndef ARK_CORE_IMPL_NUMERIC_MAX_H_
#define ARK_CORE_IMPL_NUMERIC_MAX_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Max : public Numeric {
public:
    Max(const sp<Numeric>& delegate, const sp<Numeric>& max);

    virtual float val() override;

//  [[plugin::function("max")]]
    static sp<Numeric> max(const sp<Numeric>& a, const sp<Numeric>& b);

//  [[plugin::style("max")]]
    class DECORATOR : public Builder<Numeric> {
    public:
        DECORATOR(BeanFactory& parent, const sp<Builder<Numeric>>& delegate, const String& value);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _max;
    };

private:
    sp<Numeric> _delegate;
    sp<Numeric> _max;
};

}

#endif
