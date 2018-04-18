#ifndef ARK_GRAPHICS_BASE_VEC2_H_
#define ARK_GRAPHICS_BASE_VEC2_H_

#include "core/base/api.h"
#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v2.h"
#include "graphics/forwarding.h"

namespace ark {

//[[core::class]]
class ARK_API Vec2 final : public VV2 {
public:
    Vec2() noexcept;
    Vec2(float x, float y) noexcept;
    Vec2(const V2& xy) noexcept;
//  [[script::bindings::auto]]
    Vec2(const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept;
    Vec2(float x, float y, const sp<Numeric>& xProperty, const sp<Numeric>& yProperty) noexcept;
    Vec2(VV2& other) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec2);

    virtual V2 val() override;

    friend Vec2 operator +(const Vec2& lvalue, const Vec2& rvalue);
    friend Vec2 operator -(const Vec2& lvalue, const Vec2& rvalue);
    friend Vec2 operator *(const Vec2& lvalue, const Vec2& rvalue);
    friend Vec2 operator /(const Vec2& lvalue, const Vec2& rvalue);

//  [[script::bindings::auto]]
    sp<Vec2> negative();

//  [[script::bindings::property]]
    float x() const;
//  [[script::bindings::property]]
    float y() const;
//  [[script::bindings::property]]
    V2 xy() const;

//  [[script::bindings::property]]
    void setX(float x);
//  [[script::bindings::property]]
    void setY(float y);
//  [[script::bindings::property]]
    void setXy(const V2& xy);

//  [[script::bindings::property]]
    sp<Numeric> vx() const;
//  [[script::bindings::property]]
    sp<Numeric> vy() const;

//  [[script::bindings::property]]
    void setVx(const sp<Numeric>& vx) const;
//  [[script::bindings::property]]
    void setVy(const sp<Numeric>& vy) const;

//  [[script::bindings::auto]]
    void assign(const sp<Vec2>& delegate);
//  [[script::bindings::auto]]
    void fix();

//  [[script::bindings::auto]]
    Vec2 translate(const Vec2& translation) const;
    Vec2 translate(float x, float y) const;
    Vec2 translate(ObjectPool& op, float x, float y) const;

//  [[script::bindings::auto]]
    sp<VV2> transform(const sp<Transform>& transform, const sp<VV2>& org) const;

//  [[plugin::builder]]
    class BUILDER : public Builder<Vec2> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Vec2> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x;
        sp<Builder<Numeric>> _y;

    };

//  [[plugin::builder("vec2")]]
    class VV2_BUILDER : public Builder<VV2> {
    public:
        VV2_BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<VV2> build(const sp<Scope>& args) override;

    private:
        BUILDER _impl;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Vec2> {
    public:
        DICTIONARY(BeanFactory& factory, const String str);

        virtual sp<Vec2> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y;
    };

//  [[plugin::builder::by-value]]
    class VV2_DICTIONARY : public Builder<VV2> {
    public:
        VV2_DICTIONARY(BeanFactory& parent, const String str);

        virtual sp<VV2> build(const sp<Scope>& args) override;

    private:
        DICTIONARY _delegate;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;

};

}

#endif
