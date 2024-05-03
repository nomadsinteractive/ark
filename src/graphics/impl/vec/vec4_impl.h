#ifndef ARK_GRAPHICS_IMPL_VEC_VEC4_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC4_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Vec4Impl final : public Vec4, public Holder {
public:
    Vec4Impl() noexcept;
    Vec4Impl(sp<Numeric> v) noexcept;
    Vec4Impl(float x, float y, float z, float w) noexcept;
    Vec4Impl(sp<Numeric> x, sp<Numeric> y, sp<Numeric> z, sp<Numeric> w) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec4Impl);

    virtual V4 val() override;
    virtual bool update(uint64_t timestamp) override;
    virtual void traverse(const Visitor& visitor) override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;
    const sp<NumericWrapper>& z() const;
    const sp<NumericWrapper>& w() const;

    void set(const V4& val);
    void fix();

//  [[plugin::builder]]
    class BUILDER : public Builder<Vec4> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Vec4> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _x, _y, _z, _w;
        sp<Builder<Vec4>> _value;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Vec4> {
    public:
        DICTIONARY(BeanFactory& parent, const String& str);

        virtual sp<Vec4> build(const Scope& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z, _w;
        bool _is_color;
        Color _color;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
    sp<NumericWrapper> _z;
    sp<NumericWrapper> _w;

};

}

#endif
