#ifndef ARK_GRAPHICS_BASE_ROTATION_H_
#define ARK_GRAPHICS_BASE_ROTATION_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {


//[[script::bindings::holder]]
//[[script::bindings::extends(Vec4)]]
class ARK_API Rotation : public Holder, public Vec4 {
public:
//  [[script::bindings::auto]]
    Rotation(float theta, const V3& axis = Rotation::Z_AXIS);
//  [[script::bindings::auto]]
    Rotation(const sp<Numeric>& theta, const sp<Vec3>& axis = nullptr);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Rotation);

    virtual V4 val() override;
    virtual bool update(uint64_t timestamp) override;

    virtual void traverse(const Visitor& visitor) override;

//  [[script::bindings::property]]
    const sp<Numeric>& theta() const;
//  [[script::bindings::property]]
    const sp<Vec3>& axis() const;

//  [[script::bindings::auto]]
    void setRotation(float theta, const V3& axis);
//  [[script::bindings::auto]]
    void setRotation(const sp<Numeric>& theta, const sp<Vec3>& axis);

//[[plugin::builder]]
    class BUILDER : public Builder<Rotation> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Rotation> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _theta;
        SafePtr<Builder<Vec3>> _axis;
    };

//  [[plugin::builder::by-value]]
    class DICTIONARY : public Builder<Rotation> {
    public:
        DICTIONARY(BeanFactory& factory, const String& str);

        virtual sp<Rotation> build(const Scope& args) override;

    private:
        SafePtr<Builder<Numeric>> _theta;
    };

    static const V3 Z_AXIS;

private:
    sp<Numeric> _theta;
    sp<Vec3> _axis;

    sp<Vec4> _delegate;
};

}

#endif
