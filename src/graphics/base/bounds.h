#ifndef ARK_GRAPHICS_BASE_BOUNDS_H_
#define ARK_GRAPHICS_BASE_BOUNDS_H_

#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/base/v3.h"

namespace ark {

class ARK_API Bounds : public Block {
public:
//  [[script::bindings::auto]]
    Bounds(const sp<Vec>& center, const sp<Size>& size);

//  [[script::bindings::auto]]
    bool ptin(const V& pt) const;
//  [[script::bindings::property]]
    const sp<Vec>& center() const;
//  [[script::bindings::property]]
    virtual const sp<Size>& size() override;

private:
    sp<Vec> _center;
    sp<Size> _size;

};

}

#endif
