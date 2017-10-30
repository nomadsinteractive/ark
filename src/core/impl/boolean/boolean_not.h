#ifndef ARK_CORE_IMPL_BOOLEAN_BOOLEAN_NOT_H_
#define ARK_CORE_IMPL_BOOLEAN_BOOLEAN_NOT_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class BooleanNot : public Boolean {
public:
    BooleanNot(const sp<Boolean>& delegate);

    virtual bool val() override;

//  [[plugin::builder("not")]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Boolean> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _delegate;

    };

private:
    sp<Boolean> _delegate;

};

}

#endif
