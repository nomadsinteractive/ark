#ifndef ARK_CORE_IMPL_BOOLEAN_BOOLEAN_WRAPPER_H_
#define ARK_CORE_IMPL_BOOLEAN_BOOLEAN_WRAPPER_H_

#include "core/ark.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

//[[core::class]]
class ARK_API BooleanWrapper : public Boolean {
public:
    BooleanWrapper(bool value);
    BooleanWrapper(const sp<Boolean>& delegate);
    BooleanWrapper(const BooleanWrapper& other) = default;
    BooleanWrapper(BooleanWrapper&& other) = default;

    virtual bool val() override;

    void set(bool value);
    void setDelegate(const sp<Boolean>& delegate);

//  [[plugin::builder("boolean")]]
    class BUILDER : public Builder<Boolean> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Boolean> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Boolean>> _delegate;
    };

private:
    sp<Boolean> _delegate;
    bool _value;
};

}

#endif
