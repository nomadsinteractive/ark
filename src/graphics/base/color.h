#pragma once

#include "core/base/api.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v4.h"

namespace ark {

class [[deprecated]] Color final : public Vec4 {
public:
    Color();
    Color(uint32_t value);
    Color(float r, float g, float b, float a = 1.0);
    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Color);

    sp<Numeric> r() const;
    void setR(float r);
    sp<Numeric> g() const;
    void setG(float g);
    sp<Numeric> b() const;
    void setB(float b);
    sp<Numeric> a() const;
    void setA(float a);

    V4 rgba() const;

    sp<Vec3> toVec3() const;

    uint32_t value() const;

    void reset(uint32_t value);

    bool operator == (const Color& other) const;
    bool operator != (const Color& other) const;

    V4 val() override;
    bool update(uint64_t timestamp) override;

    const sp<Vec4Impl>& wrapped() const;

private:
    sp<Vec4Impl> _wrapped;
};

}
