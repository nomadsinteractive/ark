#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/layout.h"
#include "app/traits/layout_param.h"

namespace ark {

class ARK_API LayoutUtil {
public:

    static V2 inflate(const std::vector<sp<LayoutParam>>& slots);

    static V2 place(LayoutParam::Gravity gravity, const V2& size, const Rect& available);
    static V2 place(LayoutParam::Gravity gravity, LayoutParam::FlexDirection flexDirection, const V2& size, Rect& available);

    static std::pair<float, float> calcFlowDirection(LayoutParam::JustifyContent justifyContent, float totalSpace, float childrenSpace, size_t childCount);
    static float calcItemOffsetX(LayoutParam::Align align, const Layout::Node& rootNode, Layout::Node& item);
    static float calcItemOffsetY(LayoutParam::Align align, const Layout::Node& rootNode, Layout::Node& item);

private:
    static float placeOneDimension(LayoutParam::Gravity gravity, float size, float available);

    static Rect flow(LayoutParam::FlexDirection flexDirection, const V2& size, Rect& available);
};

}
