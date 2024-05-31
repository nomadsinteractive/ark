#pragma once

#include "core/base/api.h"

#include "core/types/box.h"

namespace ark {

class ARK_API Tag final {
public:
    Tag(Box info);

//  [[script::bindings::property]]
    const Box& info() const;

private:
    Box _info;
};

}
