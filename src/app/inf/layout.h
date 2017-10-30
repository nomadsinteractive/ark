#ifndef ARK_APP_INF_LAYOUT_H_
#define ARK_APP_INF_LAYOUT_H_

#include "core/base/api.h"

#include "graphics/base/rect.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API Layout {
public:
    virtual ~Layout() = default;

    virtual void begin(LayoutParam& layoutParam) = 0;
    virtual Rect place(LayoutParam& layoutParam) = 0;
    virtual Rect end() = 0;
};

}

#endif
