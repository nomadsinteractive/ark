#ifndef ARK_CORE_IMPL_FLATABLE_FLATABLE_NUMERIC_H_
#define ARK_CORE_IMPL_FLATABLE_FLATABLE_NUMERIC_H_

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class FlatableNumeric {
public:
//  [[plugin::builder::by-value("float")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Flatable> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _numeric;
    };
};

}

#endif
