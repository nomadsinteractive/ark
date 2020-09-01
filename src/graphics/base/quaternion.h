#ifndef ARK_GRAPHICS_BASE_QUATERNION_H_
#define ARK_GRAPHICS_BASE_QUATERNION_H_

#include "core/base/api.h"
#include "core/inf/holder.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class ARK_API Quaternion : public Holder, public Vec4 {
public:
    Quaternion(const sp<Numeric>& rad, const sp<Vec3>& direction);
    Quaternion(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Quaternion);

    virtual V4 val() override;
    virtual bool update(uint64_t timestamp) override;

    virtual void traverse(const Visitor& visitor) override;

    void setRotation(const sp<Numeric>& rad, const sp<Vec3>& axis);
    void setEuler(const sp<Numeric>& pitch, const sp<Numeric>& yaw, const sp<Numeric>& roll);

private:
    sp<Vec4> _delegate;
};

}

#endif
