#ifndef ARK_CORE_IMPL_BOOLEAN_BOOLEAN_AND_H_
#define ARK_CORE_IMPL_BOOLEAN_BOOLEAN_AND_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanAnd : public Boolean {
public:
    BooleanAnd(const sp<Boolean>& a1, const sp<Boolean>& a2);

    virtual bool val() override;

//  [[plugin::builder("and")]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Boolean> build(const Scope& args) override;

    private:
        sp<Builder<Boolean>> _a1;
        sp<Builder<Boolean>> _a2;
    };

private:
    sp<Boolean> _a1;
    sp<Boolean> _a2;

};

}

#endif
