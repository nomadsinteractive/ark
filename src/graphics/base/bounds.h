#pragma once

#include "core/base/api.h"
#include "core/inf/updatable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/base/v3.h"

namespace ark {

class ARK_API Bounds : public Updatable {
public:
//  [[script::bindings::auto]]
    Bounds(const sp<Vec3>& center, const sp<Size>& size);

//  [[script::bindings::auto]]
    bool ptin(const V3& pt) const;
//  [[script::bindings::property]]
    const sp<Vec3>& center() const;

//  [[script::bindings::property]]
    const sp<Size>& size();

    virtual bool update(uint64_t timestamp) override;

private:
    sp<Vec3> _center;
    SafePtr<Size> _size;
};

}
