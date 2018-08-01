#ifndef ARK_GRAPHICS_IMPL_VEC_VEC3_H_
#define ARK_GRAPHICS_IMPL_VEC_VEC3_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Vec3Impl final : public Vec3 {
public:
    Vec3Impl() noexcept;
    Vec3Impl(float x, float y, float z) noexcept;
    Vec3Impl(const sp<Numeric>& x, const sp<Numeric>& y, const sp<Numeric>& z) noexcept;
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Vec3Impl);

    virtual V3 val() override;

    const sp<NumericWrapper>& x() const;
    const sp<NumericWrapper>& y() const;
    const sp<NumericWrapper>& z() const;

    void set(const V3& val);
    void fix();

//  [[plugin::builder("vec3")]]
    class BUILDER : public Builder<Vec3Impl> {
    public:
        BUILDER(BeanFactory& parent, const document& doc);

        virtual sp<Vec3Impl> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Vec3> {
    public:
        DICTIONARY(BeanFactory& parent, const String& str);

        virtual sp<Vec3> build(const sp<Scope>& args) override;

    private:
        sp<Builder<Numeric>> _x, _y, _z;
        V3 _v3;
    };

private:
    sp<NumericWrapper> _x;
    sp<NumericWrapper> _y;
    sp<NumericWrapper> _z;

};

}

#endif
