#pragma once

#include "core/forwarding.h"

namespace ark {

template<class T> class Loader {
public:
    virtual ~Loader() = default;

    virtual T load(const sp<Readable>& readable) = 0;

};

}
