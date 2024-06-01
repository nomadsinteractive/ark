#pragma once

#include "core/base/api.h"

#include "core/types/box.h"

namespace ark {

class ARK_API Tag final {
public:
//  [[script::bindings::auto]]
    Tag(Box info);

//  [[script::bindings::property]]
    const Box& info() const;

private:
    Box _info;
};

}
