#ifndef ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_COLOR3B_H_
#define ARK_GRAPHICS_IMPL_FLATABLE_FLATABLE_COLOR3B_H_

#include "core/inf/builder.h"
#include "core/inf/input.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class InputColor3b : public Input {
public:
    InputColor3b(const sp<Vec4>& color);

    virtual void flat(void* buf) override;
    virtual uint32_t size() override;

    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("color3b")]]
    class BUILDER : public Builder<Input> {
    public:
        BUILDER(BeanFactory& parent, const String& value);

        virtual sp<Input> build(const Scope& args) override;

    private:
        sp<Builder<Vec4>> _color;
    };

private:
    sp<Vec4> _color;

};

}

#endif
