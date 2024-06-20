#pragma once

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

//[[core::class]]
//[[script::bindings::extends(Vec4)]]
class ARK_API Color : public Vec4 {
public:
    Color();
    Color(uint32_t value);
//  [[script::bindings::auto]]
    Color(float r, float g, float b, float a = 1.0);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Color);

//  [[script::bindings::property]]
    sp<Numeric> r() const;
//  [[script::bindings::property]]
    void setR(float r);
//  [[script::bindings::property]]
    sp<Numeric> g() const;
//  [[script::bindings::property]]
    void setG(float g);
//  [[script::bindings::property]]
    sp<Numeric> b() const;
//  [[script::bindings::property]]
    void setB(float b);
//  [[script::bindings::property]]
    sp<Numeric> a() const;
//  [[script::bindings::property]]
    void setA(float a);

//  [[script::bindings::property]]
    V4 rgba() const;

//  [[script::bindings::auto]]
    sp<Vec3> toVec3() const;

//  [[script::bindings::property]]
    uint32_t value() const;
//  [[script::bindings::property]]
    void setValue(uint32_t value);

//  [[script::bindings::auto]]
    void assign(const Color& other);

    bool operator == (const Color& other) const;
    bool operator != (const Color& other) const;

    V4 val() override;
    bool update(uint64_t timestamp) override;

    const sp<Vec4Impl>& wrapped() const;

private:
    sp<Vec4Impl> _wrapped;

};

}
