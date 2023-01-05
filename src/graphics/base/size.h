#ifndef ARK_GRAPHICS_BASE_SIZE_H_
#define ARK_GRAPHICS_BASE_SIZE_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/vec/vec3_impl.h"

namespace ark {

//[[script::bindings::holder]]
//[[script::bindings::extends(Vec3)]]
class ARK_API Size : public Vec3, public Holder {
public:
    Size();
    Size(const V3& size);
//  [[script::bindings::auto]]
    Size(float width, float height, float depth = 0);
//  [[script::bindings::auto]]
    Size(sp<Numeric> width, sp<Numeric> height, sp<Numeric> depth = nullptr);

    virtual V3 val() override;
    virtual bool update(uint64_t timestamp) override;

    virtual void traverse(const Visitor& visitor) override;

    float widthAsFloat() const;
    float heightAsFloat() const;
    float depthAsFloat() const;

//  [[script::bindings::property]]
    sp<Numeric> width() const;
//  [[script::bindings::property]]
    void setWidth(float width);
//  [[script::bindings::property]]
    void setWidth(const sp<Numeric>& width);
//  [[script::bindings::property]]
    sp<Numeric> height() const;
//  [[script::bindings::property]]
    void setHeight(float height);
//  [[script::bindings::property]]
    void setHeight(const sp<Numeric>& height);
//  [[script::bindings::property]]
    sp<Numeric> depth() const;
//  [[script::bindings::property]]
    void setDepth(float depth);
//  [[script::bindings::property]]
    void setDepth(const sp<Numeric>& depth);

//  [[script::bindings::auto]]
    void set(const Size& other);
//  [[script::bindings::auto]]
    sp<Size> freeze();

    const sp<Vec3Impl>& impl() const;

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
    sp<Vec3Impl> _impl;
};

}

#endif
