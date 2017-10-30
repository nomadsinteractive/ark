#ifndef ARK_CORE_IMPL_NUMERIC_ATAN2_H_
#define ARK_CORE_IMPL_NUMERIC_ATAN2_H_

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

namespace ark {

class Atan2 : public Numeric {
public:
    Atan2(const sp<Numeric>& y, const sp<Numeric>& x);

    virtual float val() override;

//  [[plugin::function("atan2")]]
    static sp<Numeric> call(const sp<Numeric>& y, const sp<Numeric>& x);

//  [[plugin::builder("atan2")]]
    class BUILDER : public Builder<Numeric> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Numeric> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _y;
        sp<Builder<Numeric>> _x;
    };


private:
    sp<Numeric> _x;
    sp<Numeric> _y;
};

}

#endif
