#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_VEC3_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_VEC3_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableVec3 : public Flatable {
public:
    FlatableVec3(const sp<VV3>& vv3);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual uint32_t length() override;

//  [[plugin::builder::by-value("v3f")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Flatable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<VV3>> _vv3;
    };

private:
    sp<VV3> _vv3;

};

}

#endif
