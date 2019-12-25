#ifndef ARK_GRAPHICS_BASE_BOUNDS_H_
#define ARK_GRAPHICS_BASE_BOUNDS_H_

#include "core/base/api.h"
#include "core/inf/updatable.h"
#include "core/types/safe_ptr.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Bounds : public Block, public Updatable {
public:
//  [[script::bindings::auto]]
    Bounds(const sp<Vec3>& center, const sp<Size>& size);

//  [[script::bindings::auto]]
    bool ptin(const V& pt) const;
//  [[script::bindings::property]]
    const sp<Vec3>& center() const;

//  [[script::bindings::property]]
    virtual const sp<Size>& size() override;

    virtual bool update(uint64_t timestamp) override;

private:
    sp<Vec3> _center;
    SafePtr<Size> _size;
};

}

#endif
