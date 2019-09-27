#ifndef ARK_CORE_IMPL_NUMERIC_SINE_H_
#define ARK_CORE_IMPL_NUMERIC_SINE_H_

#include "core/ark.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/base/clock.h"

namespace ark {

class Sine : public Numeric {
public:
    Sine(const sp<Numeric>& rad);

    virtual float val() override;

//  [[plugin::function("sin")]]
    static sp<Numeric> call(const sp<Numeric>& x);

//  [[plugin::builder("sin")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x;
    };

private:
    sp<Numeric> _x;

};

}

#endif
