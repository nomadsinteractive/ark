#ifndef ARK_CORE_IMPL_FLATABLE_FLATABLE_INT32_H_
#define ARK_CORE_IMPL_FLATABLE_FLATABLE_INT32_H_

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class FlatableInt32 {
public:
//  [[plugin::builder::by-value("int32")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Flatable> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> _var;
    };
};

}

#endif
