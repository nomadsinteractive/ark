#ifndef ARK_GRAPHICS_BASE_SIZE_H_
#define ARK_GRAPHICS_BASE_SIZE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

//[[script::bindings::holder]]
//[[core::class]]
class ARK_API Size : public Vec3, public Holder {
public:
    Size();
    Size(float width, float height, float depth = 0);
//  [[script::bindings::auto]]
    Size(const sp<Numeric>& width, const sp<Numeric>& height, const sp<Numeric>& depth = nullptr);

    virtual V3 val() override;
    virtual bool update(uint64_t timestamp) override;

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    float width() const;
//  [[script::bindings::property]]
    void setWidth(float width);
//  [[script::bindings::property]]
    void setWidth(const sp<Numeric>& width);
//  [[script::bindings::property]]
    float height() const;
//  [[script::bindings::property]]
    void setHeight(float height);
//  [[script::bindings::property]]
    void setHeight(const sp<Numeric>& height);
//  [[script::bindings::property]]
    float depth() const;
//  [[script::bindings::property]]
    void setDepth(float depth);
//  [[script::bindings::property]]
    void setDepth(const sp<Numeric>& depth);

//  [[script::bindings::property]]
    const sp<Numeric> vwidth() const;
//  [[script::bindings::property]]
    const sp<Numeric> vheight() const;
//  [[script::bindings::property]]
    const sp<Numeric> vdepth() const;

//  [[script::bindings::auto]]
    void assign(const Size& other);
//  [[script::bindings::auto]]
    void adopt(const Size& other);

//  [[plugin::builder]]
    class BUILDER : public Builder<Size> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Size> build(const Scope& args) override;

    private:
        sp<Builder<Size>> _size;
        SafePtr<Builder<Numeric>> _width;
        SafePtr<Builder<Numeric>> _height;
        SafePtr<Builder<Numeric>> _depth;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Size> {
    public:
        DICTIONARY(BeanFactory& factory, const String& value);

        virtual sp<Size> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _width;
        sp<Builder<Numeric>> _height;
        sp<Builder<Numeric>> _depth;
    };

private:
    sp<NumericWrapper> _width;
    sp<NumericWrapper> _height;
    sp<NumericWrapper> _depth;
};

}

#endif
