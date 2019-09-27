#ifndef ARK_CORE_IMPL_NUMERIC_FOCUS_H_
#define ARK_CORE_IMPL_NUMERIC_FOCUS_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Focus : public Numeric {
public:
    Focus(const sp<Numeric>& focus, float nearest, float farest, float value);

    virtual float val() override;

//  [[plugin::builder("focus")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _target;
        sp<Builder<Numeric>> _nearest;
        sp<Builder<Numeric>> _farest;
        sp<Builder<Numeric>> _value;

    };

private:
    sp<Numeric> _target;
    float _nearest;
    float _farest;

    float _value;
};

}

#endif
