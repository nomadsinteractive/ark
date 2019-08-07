#ifndef ARK_CORE_INF_HOLDER_H_
#define ARK_CORE_INF_HOLDER_H_

#include <functional>

#include "core/base/api.h"
#include "core/forwarding.h"

namespace ark {

class ARK_API Holder {
public:
    virtual ~Holder() = default;

    typedef std::function<bool(const Box&)> Visitor;

    virtual int32_t traverse(const Visitor& visitor) = 0;
    virtual int32_t clear() = 0;
};

}

#endif
