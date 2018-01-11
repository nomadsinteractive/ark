#ifndef ARK_GRAPHICS_BASE_SIZE_H_
#define ARK_GRAPHICS_BASE_SIZE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[core::class]]
class ARK_API Size : public VV2 {
public:
    Size();
    Size(float width, float height);
//  [[script::bindings::auto]]
    Size(const sp<Numeric>& vwidth, const sp<Numeric>& vheight);

    virtual V2 val() override;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    void setWidth(float width);
//  [[script::bindings::property]]
    float height() const;
//  [[script::bindings::property]]
    void setHeight(float height);

//  [[script::bindings::property]]
    const sp<Numeric> vwidth() const;
//  [[script::bindings::property]]
    const sp<Numeric> vheight() const;

//  [[script::bindings::auto]]
    void assign(const Size& other);

//  [[plugin::builder]]
    class BUILDER : public Builder<Size> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest);

        virtual sp<Size> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Size>> _size;
        sp<Builder<Numeric>> _width;
        sp<Builder<Numeric>> _height;

    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Size> {
    public:
        DICTIONARY(BeanFactory& parent, const String& value);

        virtual sp<Size> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _width;
        sp<Builder<Numeric>> _height;
    };

private:
    sp<Scalar> _width;
    sp<Scalar> _height;

};

}

#endif
