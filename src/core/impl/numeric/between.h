#ifndef ARK_CORE_IMPL_NUMERIC_BETWEEN_H_
#define ARK_CORE_IMPL_NUMERIC_BETWEEN_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Between : public Numeric {
public:
    Between(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, const sp<Runnable>& oncross);

    virtual float val() override;

//  [[plugin::builder("between")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _min;
        sp<Builder<Numeric>> _max;
        sp<Builder<Runnable>> _oncross;
    };

private:
    sp<Numeric> _delegate;
    sp<Numeric> _min;
    sp<Numeric> _max;

    sp<Runnable> _oncross;

    bool _at_border;

};

}

#endif
