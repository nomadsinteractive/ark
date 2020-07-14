#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_MAT4F_ARRAY_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_MAT4F_ARRAY_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableMat4fArray : public Flatable {
public:
    FlatableMat4fArray(array<sp<Mat4>> array);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("mat4fv")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Flatable> build(const Scope& args) override;

    private:
        sp<Builder<ark::Array<sp<Mat4>>>> _array;
    };

private:
    array<sp<Mat4>> _array;

};

}

#endif
