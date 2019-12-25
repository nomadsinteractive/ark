#ifndef ARK_CORE_IMPL_NUMERIC_BETWEEN_H_
#define ARK_CORE_IMPL_NUMERIC_BETWEEN_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Between : public Numeric {
public:
    Between(const sp<Numeric>& delegate, const sp<Numeric>& min, const sp<Numeric>& max, const sp<Observer>& observer);

    virtual float val() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder("between")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        sp<Builder<Numeric>> _min;
        sp<Builder<Numeric>> _max;
        sp<Builder<Observer>> _observer;
    };

private:
    sp<Numeric> _delegate;
    sp<Numeric> _min;
    sp<Numeric> _max;

    sp<Observer> _observer;

    float _value;
    bool _at_border;
};

}

#endif
