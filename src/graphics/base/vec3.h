#ifndef ARK_GRAPHICS_BASE_VEC3_H_
#define ARK_GRAPHICS_BASE_VEC3_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Vec3 final : public VV3 {
public:
    Vec3() noexcept;
    Vec3(float x, float y, float z) noexcept;
//  [[script::bindings::auto]]
    Vec3(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z) noexcept;
    Vec3(const sp<VV3>& delegate) noexcept;
    Vec3(const Vec3& other) noexcept = default;
    Vec3(Vec3&& other) noexcept = default;

    Vec3& operator =(const Vec3& other) = default;
    Vec3& operator =(Vec3&& other) = default;

    friend Vec3 operator +(const Vec3& lvalue, const Vec3& rvalue);
    friend Vec3 operator -(const Vec3& lvalue, const Vec3& rvalue);
    friend Vec3 operator *(const Vec3& lvalue, const Vec3& rvalue);
    friend Vec3 operator /(const Vec3& lvalue, const Vec3& rvalue);

    virtual V3 val() override;

//  [[script::bindings::auto]]
    sp<Vec3> negative();

//  [[script::bindings::property]]
    float x() const;
//  [[script::bindings::property]]
    float y() const;
//  [[script::bindings::property]]
    float z() const;

//  [[script::bindings::property]]
    void setX(float x);
//  [[script::bindings::property]]
    void setY(float y);
//  [[script::bindings::property]]
    void setZ(float z);

//  [[script::bindings::property]]
    sp<Numeric> vx() const;
//  [[script::bindings::property]]
    sp<Numeric> vy() const;
//  [[script::bindings::property]]
    sp<Numeric> vz() const;
//  [[script::bindings::property]]
    sp<Vec2> vxy() const;

//  [[script::bindings::property]]
    void setVx(const sp<Numeric>& vx) const;
//  [[script::bindings::property]]
    void setVy(const sp<Numeric>& vy) const;
//  [[script::bindings::property]]
    void setVz(const sp<Numeric>& vz) const;

//  [[script::bindings::auto]]
    void setDelegate(const sp<VV3>& delegate);

//  [[script::bindings::auto]]
    Vec3 translate(const Vec3& translation) const;
    Vec3 translate(float x, float y, float z) const;
    Vec3 translate(ObjectPool& op, float x, float y, float z) const;

//  [[plugin::builder("vec3")]]
    class BUILDER : public Builder<Vec3> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Vec3> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z;

    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<VV3> {
    public:
        DICTIONARY(BeanFactory& parent, const String& str);

        virtual sp<VV3> build(const sp<Scope>& args) override;

    private:
        array<sp<Builder<Numeric>>> _xyz;
        V3 _v3;

    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
    sp<NumericWrapper> _z;

};

}

#endif
