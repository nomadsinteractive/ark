#ifndef ARK_CORE_INF_DIRECTORY_H_
#define ARK_CORE_INF_DIRECTORY_H_

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

template<typename T> class Dictionary {
public:
    virtual ~Dictionary() = default;

    virtual T get(const String& name) = 0;

};

}

#endif
