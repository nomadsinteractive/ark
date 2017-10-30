#ifndef ARK_CORE_IMPL_BOOLEAN_BY_TIMEOUT_H_
#define ARK_CORE_IMPL_BOOLEAN_BY_TIMEOUT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanByTimeout : public Boolean {
public:
    BooleanByTimeout(const sp<Variable<uint64_t>>& ticker, uint64_t timeout);

    virtual bool val() override;

//  [[plugin::builder("by-timeout")]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Boolean> build(const sp<Scope>& args) override;

    private:
        uint64_t _usec;
        sp<Builder<Numeric>> _timeout;
    };

private:
    sp<Variable<uint64_t>> _ticker;
    uint64_t _timeout;
};

}

#endif
