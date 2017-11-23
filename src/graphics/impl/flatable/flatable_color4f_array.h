#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_COLOR4F_ARRAY_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_COLOR4F_ARRAY_H_

#include "core/inf/builder.h"
#include "core/inf/flatable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableColor4fArray : public Flatable {
public:
    FlatableColor4fArray(const sp<Array<Color>>& colorArray);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual uint32_t length() override;

//  [[plugin::builder::by-value("color4fv")]]
    class BUILDER : public Builder<Flatable> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Flatable> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Array<Color>>> _color_array;
    };

private:
    sp<Array<Color>> _color_array;

};

}

#endif
