#ifndef ARK_CORE_IMPL_NUMERIC_SCALAR_H_
#define ARK_CORE_IMPL_NUMERIC_SCALAR_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/forwarding.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

namespace ark {

class ARK_API Scalar : public Numeric {
public:
    Scalar(float value);
//  [[script::bindings::auto]]
    Scalar(const sp<Numeric>& delegate);
    Scalar(const Scalar& other) = default;
    Scalar(Scalar&& other) = default;

//  [[script::bindings::auto]]
    virtual float val() override;

    void assign(const sp<Scalar>& other);
//  [[script::bindings::auto]]
    void set(float value);
//  [[script::bindings::auto]]
    void fix();

//  [[script::bindings::property]]
    const sp<Numeric>& delegate();
//  [[script::bindings::property]]
    void setDelegate(const sp<Numeric>& delegate);

//  [[plugin::builder("numeric")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _delegate;
        float _value;
    };

private:
    void deferedUnref();

private:
    sp<Numeric> _delegate;
    float _value;
};

}

#endif
