#ifndef ARK_GRAPHICS_BASE_VEC4_H_
#define ARK_GRAPHICS_BASE_VEC4_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[core::class]]
class ARK_API Vec4 final : public VV4 {
public:
    Vec4() noexcept;
    Vec4(float x, float y, float z, float w) noexcept;
//  [[script::bindings::auto]]
    Vec4(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z, const sp<Numeric>& w) noexcept;
    Vec4(const sp<VV4>& delegate) noexcept;
    Vec4(const Vec4& other) noexcept = default;
    Vec4(Vec4&& other) noexcept = default;

    Vec4& operator =(const Vec4& other) = default;
    Vec4& operator =(Vec4&& other) = default;

    friend Vec4 operator +(const Vec4& lvalue, const Vec4& rvalue);
    friend Vec4 operator -(const Vec4& lvalue, const Vec4& rvalue);
    friend Vec4 operator *(const Vec4& lvalue, const Vec4& rvalue);
    friend Vec4 operator /(const Vec4& lvalue, const Vec4& rvalue);

    virtual V4 val() override;

//  [[script::bindings::auto]]
    sp<Vec4> negative();

//  [[script::bindings::property]]
    float x() const;
//  [[script::bindings::property]]
    float y() const;
//  [[script::bindings::property]]
    float z() const;
//  [[script::bindings::property]]
    float w() const;

//  [[script::bindings::property]]
    void setX(float x);
//  [[script::bindings::property]]
    void setY(float y);
//  [[script::bindings::property]]
    void setZ(float z);
//  [[script::bindings::property]]
    void setW(float w);

//  [[script::bindings::property]]
    sp<Numeric> vx() const;
//  [[script::bindings::property]]
    sp<Numeric> vy() const;
//  [[script::bindings::property]]
    sp<Numeric> vz() const;
//  [[script::bindings::property]]
    sp<Numeric> vw() const;

//  [[script::bindings::property]]
    void setVx(const sp<Numeric>& vx) const;
//  [[script::bindings::property]]
    void setVy(const sp<Numeric>& vy) const;
//  [[script::bindings::property]]
    void setVz(const sp<Numeric>& vz) const;
//  [[script::bindings::property]]
    void setVw(const sp<Numeric>& vz) const;

//  [[script::bindings::auto]]
    void setDelegate(const sp<VV4>& delegate);

//  [[plugin::builder("vec4")]]
    class BUILDER : public Builder<Vec4> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Vec4> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z, _w;

    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<VV4> {
    public:
        DICTIONARY(BeanFactory& parent, const String& str);

        virtual sp<VV4> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z, _w;
        V4 _v4;
        bool _is_color;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
    sp<NumericWrapper> _z;
    sp<NumericWrapper> _w;

};

}

#endif
