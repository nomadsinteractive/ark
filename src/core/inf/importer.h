#pragma once

#include "core/forwarding.h"

namespace ark {

template<typename T> class Importer {
public:
    virtual ~Importer() = default;

    virtual void import(T& obj, const sp<Readable>& src) = 0;
};

}
