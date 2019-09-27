#ifndef ARK_CORE_IMPL_BOOLEAN_BY_TIMEOUT_H_
#define ARK_CORE_IMPL_BOOLEAN_BY_TIMEOUT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanByTimeout : public Boolean {
public:
    BooleanByTimeout(const sp<Numeric>& ticker, float timeout);

    virtual bool val() override;

//  [[plugin::builder("by-timeout")]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Boolean> build(const Scope& args) override;

    private:
        sp<Builder<Duration>> _duration;
        float _sec;
        sp<Builder<Numeric>> _timeout;
    };

private:
    sp<Numeric> _duration;
    float _timeout;
};

}

#endif
