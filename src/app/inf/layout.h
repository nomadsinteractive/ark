#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include <vector>

#include "core/base/api.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Layout {
public:
    virtual ~Layout() = default;

    virtual V2 inflate(const std::vector<sp<LayoutParam>>& slots) = 0;
    virtual std::vector<V2> place(const std::vector<sp<LayoutParam>>& slots, const LayoutParam& parent, const V2& contentSize) = 0;

};

}

#endif
