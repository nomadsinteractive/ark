#ifndef ARK_GRAPHICS_IMPL_VEC_VEC4_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC4_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/color.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Vec4Impl final : public Vec4 {
public:
    Vec4Impl() noexcept;
    Vec4Impl(float x, float y, float z, float w) noexcept;
    Vec4Impl(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec4Impl);

    virtual V4 val() override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;
    const sp<NumericWrapper>& z() const;
    const sp<NumericWrapper>& w() const;

    void set(const V4& val);
    void fix();

//  [[plugin::builder("vec4")]]
    class BUILDER : public Builder<Vec4> {
    public:
        BUILDER(BeanFactory& parent, const document& manifest);

        virtual sp<Vec4> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z, _w;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Vec4> {
    public:
        DICTIONARY(BeanFactory& parent, const String& str);

        virtual sp<Vec4> build(const sp<Scope>& args) override;

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
