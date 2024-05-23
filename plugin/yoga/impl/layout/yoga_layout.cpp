#include "yoga/impl/layout/yoga_layout.h"

#include "core/ark.h"
#include "core/types/global.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"

#include "app/view/layout_param.h"
#include "app/view/view_hierarchy.h"

namespace ark::plugin::yoga {

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
    : _yg_node(nullptr)
{
}

YogaLayout::~YogaLayout()
{
    if(_yg_node)
        YGNodeFreeRecursive(_yg_node);
}

void YogaLayout::inflate(sp<Node> rootNode)
{
    if(_yg_node)
        YGNodeFreeRecursive(_yg_node);

    _yg_node = doInflate(Global<YogaConfig>(), rootNode, nullptr);
    _root_node = std::move(rootNode);
}

bool YogaLayout::update(uint64_t timestamp)
{
    if(!_root_node)
        return false;

    const LayoutParam& layoutParam = _root_node->_layout_param;
    ASSERT(layoutParam.widthType() != LayoutParam::LENGTH_TYPE_PERCENTAGE && layoutParam.heightType() != LayoutParam::LENGTH_TYPE_PERCENTAGE);
    doUpdate(_root_node, timestamp);
    YGNodeCalculateLayout(_yg_node, layoutParam.size()->widthAsFloat(), layoutParam.size()->heightAsFloat(), YGDirectionLTR);
    updateLayoutResult(_root_node);
    return YGNodeIsDirty(_yg_node);
}

template<typename T> Optional<T> updateVar(uint64_t timestamp, Variable<T>& var)
{
    if(!timestamp || var.update(timestamp))
        return var.val();
    return Optional<T>();
}

void YogaLayout::applyLayoutParam(const LayoutParam& layoutParam, YGNodeRef node, uint64_t timestamp)
{
    const Optional<float> width = updateVar<float>(timestamp, layoutParam.width());
    if(width)
    {
        if(layoutParam.widthType() == LayoutParam::LENGTH_TYPE_AUTO)
            YGNodeStyleSetWidthAuto(node);
        else if(layoutParam.widthType() == LayoutParam::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetWidth(node, width.value());
        else
        {
            ASSERT(layoutParam.widthType() == LayoutParam::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetWidthPercent(node, width.value());
        }
    }

    const Optional<float> height = updateVar<float>(timestamp, layoutParam.height());
    if(height)
    {
        if(layoutParam.heightType() == LayoutParam::LENGTH_TYPE_AUTO)
            YGNodeStyleSetHeightAuto(node);
        else if(layoutParam.heightType() == LayoutParam::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetHeight(node, height.value());
        else
        {
            ASSERT(layoutParam.heightType() == LayoutParam::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetHeightPercent(node, height.value());
        }
    }

    YGNodeStyleSetFlexDirection(node, toYGFlexDirection(layoutParam.flexDirection()));
    YGNodeStyleSetFlexGrow(node, layoutParam.flexGrow());

    const Optional<float> flexBasis = updateVar<float>(timestamp, layoutParam.flexBasis());
    if(flexBasis)
    {
        if(layoutParam.flexBasisType() == LayoutParam::LENGTH_TYPE_AUTO)
            YGNodeStyleSetFlexBasisAuto(node);
        else if(layoutParam.flexBasisType() == LayoutParam::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetFlexBasis(node, flexBasis.value());
        else
        {
            ASSERT(layoutParam.flexBasisType() == LayoutParam::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetFlexBasisPercent(node, flexBasis.value());
        }
    }

    YGNodeStyleSetFlexWrap(node, toYGWrap(layoutParam.flexWrap()));
    YGNodeStyleSetJustifyContent(node, toYGJustify(layoutParam.justifyContent()));
    YGNodeStyleSetAlignItems(node, toYGAlign(layoutParam.alignItems()));
    YGNodeStyleSetAlignSelf(node, toYGAlign(layoutParam.alignSelf()));
    YGNodeStyleSetAlignContent(node, toYGAlign(layoutParam.alignContent()));

    if(layoutParam.margins())
        if(!timestamp || layoutParam.margins().update(timestamp))
        {
            const V4 margins = layoutParam.margins().val();
            YGNodeStyleSetMargin(node, YGEdgeTop, margins.x());
            YGNodeStyleSetMargin(node, YGEdgeRight, margins.y());
            YGNodeStyleSetMargin(node, YGEdgeBottom, margins.z());
            YGNodeStyleSetMargin(node, YGEdgeLeft, margins.w());
        }

    if(layoutParam.paddings())
        if(!timestamp || layoutParam.paddings().update(timestamp))
        {
            const V4 paddings = layoutParam.paddings().val();
            YGNodeStyleSetPadding(node, YGEdgeTop, paddings.x());
            YGNodeStyleSetPadding(node, YGEdgeRight, paddings.y());
            YGNodeStyleSetPadding(node, YGEdgeBottom, paddings.z());
            YGNodeStyleSetPadding(node, YGEdgeLeft, paddings.w());
        }
}

void YogaLayout::updateLayoutResult(Node& layoutNode)
{
    YGNodeRef ygNode = reinterpret_cast<YGNodeRef>(layoutNode._tag);
    layoutNode.setPaddings(V4(YGNodeLayoutGetPadding(ygNode, YGEdgeTop), YGNodeLayoutGetPadding(ygNode, YGEdgeRight),
                              YGNodeLayoutGetPadding(ygNode, YGEdgeBottom), YGNodeLayoutGetPadding(ygNode, YGEdgeLeft)));
    layoutNode.setOffsetPosition(V2(YGNodeLayoutGetLeft(ygNode), YGNodeLayoutGetTop(ygNode)));
    layoutNode.setSize(V2(YGNodeLayoutGetWidth(ygNode), YGNodeLayoutGetHeight(ygNode)));

    if(layoutNode._view_hierarchy)
        for(const ViewHierarchy::Slot& i : layoutNode._view_hierarchy->updateSlots())
            updateLayoutResult(i.layoutNode());
}

YGNodeRef YogaLayout::doInflate(const YogaConfig& config, Node& layoutNode, YGNodeRef parentNode)
{
    YGNodeRef ygNode = config.newNode();
    layoutNode._tag = reinterpret_cast<void*>(ygNode);

    if(parentNode)
        YGNodeInsertChild(parentNode, ygNode, YGNodeGetChildCount(parentNode));

    if(layoutNode._view_hierarchy)
        for(const ViewHierarchy::Slot& i : layoutNode._view_hierarchy->updateSlots())
            doInflate(config, i.layoutNode(), ygNode);

    return ygNode;
}

void YogaLayout::doUpdate(Node& layoutNode, uint64_t timestamp)
{
    YGNodeRef ygNode = reinterpret_cast<YGNodeRef>(layoutNode._tag);

    if(layoutNode._layout_param)
        applyLayoutParam(layoutNode._layout_param, ygNode, timestamp);

    if(layoutNode._view_hierarchy)
        for(const ViewHierarchy::Slot& i : layoutNode._view_hierarchy->updateSlots())
            doUpdate(i.layoutNode(), timestamp);
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
    YGConfigSetUseWebDefaults(_config, true);
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
