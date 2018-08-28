#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V4F_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V4F_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableV4f : public Flatable {
public:
    FlatableV4f(const sp<Vec4>& vec4);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual uint32_t length() override;

//  [[plugin::builder::by-value("v4f")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Flatable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Vec4>> _vec4;
    };

private:
    sp<Vec4> _vec4;

};

}

#endif
