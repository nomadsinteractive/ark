#pragma once

#include "core/base/api.h"

#include "core/types/box.h"

namespace ark {

class ARK_API WithTag final {
public:
//  [[script::bindings::auto]]
    WithTag(Box tag);

//  [[script::bindings::property]]
    const Box& tag() const;

private:
    Box _info;
};

}
