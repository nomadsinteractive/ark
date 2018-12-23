#ifndef ARK_CORE_BASE_EXPECTATION_H_
#define ARK_CORE_BASE_EXPECTATION_H_

#include "core/base/api.h"
#include "core/base/observer.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Expectation : public Numeric {
public:
//[[script::bindings::auto]]
    Expectation(const sp<Numeric>& expectation, const sp<Runnable>& onfire = nullptr, bool fireOnce = true);

//[[script::bindings::property]]
    virtual float val() override;
//[[script::bindings::property]]
    void setVal(float val);

    void fire();

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Expectation> {
    public:
        DICTIONARY(BeanFactory& factory, const String str);

        virtual sp<Expectation> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _expectation;
        sp<Builder<Runnable>> _onfire;
    };

private:
    sp<Numeric> _expectation;
    Observer _observer;
};

}

#endif
