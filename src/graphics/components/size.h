#pragma once

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/impl/vec/vec3_impl.h"

namespace ark {

//[[script::bindings::extends(Vec3)]]
class ARK_API Size final : public Vec3 {
public:
    Size();
    Size(const V3& size);
//  [[script::bindings::auto]]
    Size(float width, float height, float depth = 0);
//  [[script::bindings::auto]]
    Size(sp<Numeric> width, sp<Numeric> height, sp<Numeric> depth = nullptr);

    V3 val() override;
    bool update(uint64_t timestamp) override;

    float widthAsFloat() const;
    float heightAsFloat() const;
    float depthAsFloat() const;

//  [[script::bindings::property]]
    sp<Numeric> width() const;
//  [[script::bindings::property]]
    void setWidth(float width);
//  [[script::bindings::property]]
    void setWidth(sp<Numeric> width);
//  [[script::bindings::property]]
    sp<Numeric> height() const;
//  [[script::bindings::property]]
    void setHeight(float height);
//  [[script::bindings::property]]
    void setHeight(sp<Numeric> height);
//  [[script::bindings::property]]
    sp<Numeric> depth() const;
//  [[script::bindings::property]]
    void setDepth(float depth);
//  [[script::bindings::property]]
    void setDepth(sp<Numeric> depth);

//  [[script::bindings::auto]]
    void reset(const Size& other);
//  [[script::bindings::auto]]
    sp<Size> freeze();

    const sp<Vec3Impl>& impl() const;

//  [[plugin::builder::by-value]]
    class DICTIONARY final : public Builder<Size> {
    public:
        DICTIONARY(BeanFactory& factory, const String& expr);

        sp<Size> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _width, _height, _depth;
        String _expr;
    };

//  [[plugin::builder]]
    class BUILDER final : public Builder<Size> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<Size> build(const Scope& args) override;

    private:
        sp<Builder<Size>> _size;
        SafeBuilder<Numeric> _width;
        SafeBuilder<Numeric> _height;
        SafeBuilder<Numeric> _depth;
    };

private:
    sp<Vec3Impl> _impl;
};

}
