#pragma once

#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

namespace ark {

class Position {
public:
//  [[script::bindings::auto]]
    Position(sp<Vec3> xyz);

//  [[script::bindings::auto]]
    const sp<Vec3>& xyz() const;

private:
    sp<Vec3> _xyz;
};

} // ark
