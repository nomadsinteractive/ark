#ifndef ARK_CORE_BASE_EXPECTATION_H_
#define ARK_CORE_BASE_EXPECTATION_H_

#include "core/base/api.h"
#include "core/base/notifier.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[core::class]]
class ARK_API Expectation : public Numeric {
public:
//[[script::bindings::auto]]
    Expectation(const sp<Numeric>& expectation, const sp<Runnable>& onfire);

    virtual float val() override;

    void fire();
    void fireOnce();

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
    Notifier _onfire;
};

}

#endif
