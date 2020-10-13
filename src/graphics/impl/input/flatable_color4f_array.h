#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_COLOR4F_ARRAY_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_COLOR4F_ARRAY_H_

#include "core/inf/builder.h"
#include "core/inf/input.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class FlatableColor4fArray : public Input {
public:
    FlatableColor4fArray(const sp<ark::Array<Color>>& colorArray);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("color4fv")]]
    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Input> build(const Scope& args) override;

    private:
        sp<Builder<ark::Array<Color>>> _color_array;
    };

private:
    sp<ark::Array<Color>> _color_array;

};

}

#endif
