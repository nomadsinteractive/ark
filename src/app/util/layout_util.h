#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/layout.h"
#include "graphics/components/layout_param.h"

namespace ark {

class ARK_API LayoutUtil {
public:

    static std::pair<float, float> calcFlowDirection(LayoutParam::JustifyContent justifyContent, float totalSpace, float childrenSpace, size_t childCount);

    static LayoutParam::Align toAlign(LayoutParam::JustifyContent justifyContent);

    static float calcItemOffsetX(LayoutParam::Align align, const Layout::Node& rootNode, Layout::Node& item);
    static float calcItemOffsetY(LayoutParam::Align align, const Layout::Node& rootNode, Layout::Node& item);

private:
    static Rect flow(LayoutParam::FlexDirection flexDirection, const V2& size, Rect& available);
};

}
