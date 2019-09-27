#ifndef ARK_CORE_IMPL_NUMERIC_COSINE_H_
#define ARK_CORE_IMPL_NUMERIC_COSINE_H_

#include "core/ark.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/weak_ptr.h"
#include "core/base/clock.h"

namespace ark {

class Cosine : public Numeric {
public:
    Cosine(const sp<Numeric>& x);

    virtual float val() override;

//  [[plugin::function("cos")]]
    static sp<Numeric> call(const sp<Numeric>& x);

//  [[plugin::builder("cos")]]
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
