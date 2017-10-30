#ifndef ARK_CORE_IMPL_NUMERIC_PYTHAGOREAN_H_
#define ARK_CORE_IMPL_NUMERIC_PYTHAGOREAN_H_

#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Pythagorean : public Numeric {
public:
    Pythagorean(const sp<Numeric>& a1, const sp<Numeric>& a2);

    virtual float val() override;

//  [[plugin::builder("pythagorean")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& beanFactory, const document& manifest);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _a1;
        sp<Builder<Numeric>> _a2;
    };

private:
    sp<Numeric> _a1;
    sp<Numeric> _a2;
};

}

#endif
