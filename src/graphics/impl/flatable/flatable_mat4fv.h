#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_MAT4FV_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_MAT4FV_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/base/identifier.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableMat4fv : public Flatable {
public:
    FlatableMat4fv(array<sp<Mat4>> array);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("mat4fv")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& factory, const String& value);

        virtual sp<Flatable> build(const Scope& args) override;

    private:
        Identifier _id;
        sp<Builder<Array<sp<Mat4>>>> _array;
    };

private:
    array<sp<Mat4>> _array;

};

}

#endif
