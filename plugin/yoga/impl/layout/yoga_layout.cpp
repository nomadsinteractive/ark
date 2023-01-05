#include "yoga/impl/layout/yoga_layout.h"

#include "core/ark.h"
#include "core/types/global.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"

#include "app/view/layout_param.h"

namespace ark {
namespace plugin {
namespace yoga {

static YGFlexDirection toYGFlexDirection(LayoutParam::FlexDirection flexDirection)
{
    switch(flexDirection)
    {
        case LayoutParam::FLEX_DIRECTION_COLUMN:
            return YGFlexDirectionColumn;
        case LayoutParam::FLEX_DIRECTION_COLUMN_REVERSE:
            return YGFlexDirectionColumnReverse;
        case LayoutParam::FLEX_DIRECTION_ROW:
            return YGFlexDirectionRow;
        case LayoutParam::FLEX_DIRECTION_ROW_REVERSE:
            return YGFlexDirectionRowReverse;
    }
    DFATAL("Unknow FlexDirection: %d", flexDirection);
    return YGFlexDirectionColumn;
}

static YGWrap toYGWrap(LayoutParam::FlexWrap flexWrap)
{
    switch(flexWrap)
    {
        case LayoutParam::FLEX_WRAP_NOWRAP:
            return YGWrapNoWrap;
        case LayoutParam::FLEX_WRAP_WRAP:
            return YGWrapWrap;
        case LayoutParam::FLEX_WRAP_WRAP_REVERSE:
            return YGWrapWrapReverse;
    }
    DFATAL("Unknow FlexWrap: %d", flexWrap);
    return YGWrapNoWrap;
}

static YGJustify toYGJustify(LayoutParam::JustifyContent justifyContent)
{
    switch(justifyContent)
    {
    case LayoutParam::JUSTIFY_CONTENT_FLEX_START:
        return YGJustifyFlexStart;
    case LayoutParam::JUSTIFY_CONTENT_FLEX_END:
        return YGJustifyFlexEnd;
    case LayoutParam::JUSTIFY_CONTENT_CENTER:
        return YGJustifyCenter;
    case LayoutParam::JUSTIFY_CONTENT_SPACE_BETWEEN:
        return YGJustifySpaceBetween;
    case LayoutParam::JUSTIFY_CONTENT_SPACE_AROUND:
        return YGJustifySpaceAround;
    case LayoutParam::JUSTIFY_CONTENT_SPACE_EVENLY:
        return YGJustifySpaceEvenly;
    }
    DFATAL("Unknow JustifyContent: %d", justifyContent);
    return YGJustifyFlexStart;
}

static YGAlign toYGAlign(LayoutParam::Align align)
{
    switch(align)
    {
    case LayoutParam::ALIGN_AUTO:
        return YGAlignAuto;
    case LayoutParam::ALIGN_FLEX_START:
        return YGAlignFlexStart;
    case LayoutParam::ALIGN_CENTER:
        return YGAlignCenter;
    case LayoutParam::ALIGN_FLEX_END:
        return YGAlignFlexEnd;
    case LayoutParam::ALIGN_STRETCH:
        return YGAlignStretch;
    case LayoutParam::ALIGN_BASELINE:
        return YGAlignBaseline;
    case LayoutParam::ALIGN_SPACE_BETWEEN:
        return YGAlignSpaceBetween;
    case LayoutParam::ALIGN_SPACE_AROUND:
        return YGAlignSpaceAround;
    }
    return YGAlignStretch;
}

YogaLayout::YogaLayout()
    : _root_node(nullptr)
{
}

YogaLayout::~YogaLayout()
{
    if(_root_node)
        YGNodeFreeRecursive(_root_node);
}

void YogaLayout::inflate(Node& rootNode)
{
    if(_root_node)
        YGNodeFreeRecursive(_root_node);

    _root_node = doInflate(Global<YogaConfig>(), rootNode, nullptr);
}

void YogaLayout::place(Node& rootNode)
{

}

void YogaLayout::applyLayoutParam(const LayoutParam& layoutParam, YGNodeRef node)
{
    const LayoutParam::Length& width = layoutParam.width();
    if(width.isAuto())
        YGNodeStyleSetWidthAuto(node);
    else if(width.isPixel())
        YGNodeStyleSetWidth(node, width._value);
    else
    {
        ASSERT(width.isPercentage());
        YGNodeStyleSetWidthPercent(node, width._value);
    }

    const LayoutParam::Length& height = layoutParam.height();
    if(height.isAuto())
        YGNodeStyleSetHeightAuto(node);
    else if(height.isPixel())
        YGNodeStyleSetHeight(node, height._value);
    else
    {
        ASSERT(height.isPercentage());
        YGNodeStyleSetHeightPercent(node, height._value);
    }

    YGNodeStyleSetFlexDirection(node, toYGFlexDirection(layoutParam.flexDirection()));
    YGNodeStyleSetFlexGrow(node, layoutParam.flexGrow());
//    YGNodeStyleSetFlexBasis
    YGNodeStyleSetFlexWrap(node, toYGWrap(layoutParam.flexWrap()));
    YGNodeStyleSetJustifyContent(node, toYGJustify(layoutParam.justifyContent()));
    YGNodeStyleSetAlignItems(node, toYGAlign(layoutParam.alignItems()));
    YGNodeStyleSetAlignSelf(node, toYGAlign(layoutParam.alignSelf()));
    YGNodeStyleSetAlignContent(node, toYGAlign(layoutParam.alignContent()));

    if(layoutParam.margins())
    {
        const V4 margins = layoutParam.margins().val();
        YGNodeStyleSetMargin(node, YGEdgeTop, margins.x());
        YGNodeStyleSetMargin(node, YGEdgeRight, margins.y());
        YGNodeStyleSetMargin(node, YGEdgeBottom, margins.z());
        YGNodeStyleSetMargin(node, YGEdgeLeft, margins.w());
    }

    if(layoutParam.paddings())
    {
        const V4 paddings = layoutParam.paddings().val();
        YGNodeStyleSetPadding(node, YGEdgeTop, paddings.x());
        YGNodeStyleSetPadding(node, YGEdgeRight, paddings.y());
        YGNodeStyleSetPadding(node, YGEdgeBottom, paddings.z());
        YGNodeStyleSetPadding(node, YGEdgeLeft, paddings.w());
    }
}

YGNodeRef YogaLayout::doInflate(const Global<YogaLayout::YogaConfig>& config, Node& layoutNode, YGNodeRef parentNode)
{
    YGNodeRef ygNode = config->newNode();
    layoutNode._tag = reinterpret_cast<void*>(ygNode);

    if(layoutNode._layout_param)
        applyLayoutParam(layoutNode._layout_param, ygNode);

    if(parentNode)
        YGNodeInsertChild(parentNode, ygNode, 0);

    for(Node& i : layoutNode._child_nodes)
        doInflate(config, i, ygNode);

    return ygNode;
}

YogaLayout::BUILDER::BUILDER(BeanFactory& factory)
{
}

sp<LayoutV3> YogaLayout::BUILDER::build(const Scope& args)
{
    return sp<YogaLayout>::make();
}

YogaLayout::YogaConfig::YogaConfig()
    : _config(YGConfigNew())
{
}

YogaLayout::YogaConfig::~YogaConfig()
{
    YGConfigFree(_config);
}

YGNodeRef YogaLayout::YogaConfig::newNode() const
{
    return YGNodeNewWithConfig(_config);
}

}
}
}
