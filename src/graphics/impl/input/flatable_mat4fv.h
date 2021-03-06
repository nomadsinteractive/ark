#ifndef ARK_GRAPHICS_IMPL_INPUT_INPUT_MAT4FV_H_
#define ARK_GRAPHICS_IMPL_INPUT_INPUT_MAT4FV_H_

#include "core/inf/builder.h"
#include "core/inf/input.h"
#include "core/base/identifier.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableMat4fv : public Input {
public:
    FlatableMat4fv(array<sp<Mat4>> array);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("mat4fv")]]
    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Input> build(const Scope& args) override;

    private:
        Identifier _id;
        sp<Builder<Array<sp<Mat4>>>> _array;
    };

private:
    array<sp<Mat4>> _array;

};

}

#endif
