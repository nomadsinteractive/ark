#ifndef ARK_CORE_IMPL_INPUT_INPUT_INT32_H_
#define ARK_CORE_IMPL_INPUT_INPUT_INT32_H_

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class InputInt32 {
public:
//  [[plugin::builder::by-value("int")]]
    class BUILDER : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Integer>> _var;
    };
};

}

#endif
