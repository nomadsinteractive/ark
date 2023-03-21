#ifndef ARK_CORE_IMPL_INPUT_INPUT_NUMERIC_H_
#define ARK_CORE_IMPL_INPUT_INPUT_NUMERIC_H_

#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class InputNumeric {
public:
//  [[plugin::builder::by-value("float")]]
    class BUILDER : public Builder<Uploader> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Uploader> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _numeric;
    };
};

}

#endif
