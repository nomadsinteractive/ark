#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V2F_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_V2F_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableV2f : public Flatable {
public:
    FlatableV2f(const sp<VV2>& vv2);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;

//  [[plugin::builder::by-value("v2f")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Flatable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<VV2>> _vv2;
    };

private:
    sp<VV2> _vv2;

};

}

#endif
