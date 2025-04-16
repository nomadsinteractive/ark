#pragma once

#include "core/forwarding.h"

namespace ark {

class Asset {
public:
    virtual ~Asset() = default;

    virtual sp<Readable> open() = 0;
    virtual String location() = 0;
};

}
