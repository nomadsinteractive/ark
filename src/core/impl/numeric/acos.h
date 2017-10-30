#ifndef ARK_CORE_IMPL_NUMERIC_ACOS_H_
#define ARK_CORE_IMPL_NUMERIC_ACOS_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Acos : public Numeric {
public:
    Acos(const sp<Numeric>& x);

    virtual float val() override;

//  [[plugin::function("acos")]]
    static sp<Numeric> call(const sp<Numeric>& x);

//  [[plugin::builder("acos")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x;
    };


private:
    sp<Numeric> _x;
};

}

#endif
