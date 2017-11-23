#ifndef ARK_CORE_IMPL_FLATABLE_FLATABLE_UINT8_H_
#define ARK_CORE_IMPL_FLATABLE_FLATABLE_UINT8_H_

#include "core/inf/flatable.h"
#include "core/inf/builder.h"

#include "core/types/shared_ptr.h"

namespace ark {

class FlatableUint8 : public Flatable {
public:
    FlatableUint8(const sp<Numeric>& numeric);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual uint32_t length() override;

//  [[plugin::builder::by-value("uint8")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Flatable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _numeric;
    };

private:
    sp<Numeric> _numeric;

};

}

#endif
