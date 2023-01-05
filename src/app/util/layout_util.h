#ifndef ARK_APP_UTIL_LAYOUT_UTIL_H_
#define ARK_APP_UTIL_LAYOUT_UTIL_H_

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/view/layout_param.h"

namespace ark {

class ARK_API LayoutUtil {
public:

    static V2 inflate(const std::vector<sp<LayoutParam>>& slots);

    static V2 place(LayoutParam::Gravity gravity, const V2& size, const Rect& available);
    static V2 place(LayoutParam::Gravity gravity, LayoutParam::FlexDirection flexFlow, const V2& size, Rect& available);

private:
    static float placeOneDimension(LayoutParam::Gravity gravity, float size, float available);

    static Rect flow(LayoutParam::FlexDirection flexFlow, const V2& size, Rect& available);
};

}

#endif
