#ifndef ARK_GRAPHICS_IMPL_INPUT_INPUT_V4FV_H_
#define ARK_GRAPHICS_IMPL_INPUT_INPUT_V4FV_H_

#include "core/inf/builder.h"
#include "core/impl/input/input_variable_array.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class InputV4fv {
public:
//  [[plugin::builder::by-value("v4fv")]]
    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Input> build(const Scope& args) override;

    private:
        InputVariableArray<V4>::BUILDER _impl;
    };
};

}

#endif
