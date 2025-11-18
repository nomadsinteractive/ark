#include "yoga/impl/layout/yoga_layout.h"

#include <yoga/YGNode.h>
#include <yoga/Yoga.h>

#include "core/ark.h"
#include "core/types/global.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"
#include "graphics/components/layout_param.h"

#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark::plugin::yoga {

namespace {

YGFlexDirection toYGFlexDirection(const LayoutParam::FlexDirection flexDirection)
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

YGWrap toYGWrap(const LayoutParam::FlexWrap flexWrap)
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

YGJustify toYGJustify(const LayoutParam::JustifyContent justifyContent)
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

YGAlign toYGAlign(const LayoutParam::Align align)
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

class YogaConfig {
public:
    YogaConfig()
        : _config(YGConfigNew()) {
        YGConfigSetUseWebDefaults(_config, true);
    }
    ~YogaConfig() {
        YGConfigFree(_config);
    }

    YGNodeRef newNode() const {
        return YGNodeNewWithConfig(_config);
    }

private:
    YGConfigRef _config;
};

YGNodeRef doInflate(const YogaConfig& config, const Layout::Hierarchy& hierarchy, YGNodeRef parentNode)
{
    const YGNodeRef ygNode = config.newNode();
    hierarchy._node->_tag = ygNode;

    if(parentNode)
        YGNodeInsertChild(parentNode, ygNode, YGNodeGetChildCount(parentNode));

    for(const Layout::Hierarchy& i : hierarchy._child_nodes)
        doInflate(config, i, ygNode);

    return ygNode;
}

template<typename T, typename U> Optional<T> updateVar(uint32_t tick, U& var, const bool force)
{
    if(var.update(tick) || force)
        return {var.val()};
    return {};
}

bool updateLayoutParam(const Layout::Node& layoutNode, const YGNodeRef node, const uint32_t tick, const bool force)
{
    const LayoutParam& layoutParam = layoutNode._layout_param;

    if(layoutParam.width().isAuto())
    {
        if(layoutNode.autoWidth())
        {
            if(const Optional<float> width = updateVar<float>(tick, *layoutNode.autoWidth(), force))
                YGNodeStyleSetWidth(node, width.value());
        }
        else
            YGNodeStyleSetWidthAuto(node);
    }
    else if(const Optional<float> width = updateVar<float>(tick, layoutParam.width().value(), force))
    {
        if(layoutParam.width().type() == LayoutLength::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetWidth(node, width.value());
        else
        {
            ASSERT(layoutParam.width().type() == LayoutLength::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetWidthPercent(node, width.value());
        }
    }

    if(layoutParam.height().isAuto())
    {
        if(layoutNode.autoHeight())
        {
            if(const Optional<float> height = updateVar<float>(tick, *layoutNode.autoHeight(), force))
                YGNodeStyleSetHeight(node, height.value());
        }
        else
            YGNodeStyleSetHeightAuto(node);
    }
    else if(const Optional<float> height = updateVar<float>(tick, layoutParam.height().value(), force))
    {
        if(layoutParam.height().type() == LayoutLength::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetHeight(node, height.value());
        else
        {
            ASSERT(layoutParam.height().type() == LayoutLength::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetHeightPercent(node, height.value());
        }
    }

    YGNodeStyleSetFlexDirection(node, toYGFlexDirection(layoutParam.flexDirection()));
    YGNodeStyleSetFlexGrow(node, layoutParam.flexGrow());

    if(const Optional<float> flexBasis = updateVar<float>(tick, layoutParam.flexBasis().value(), force))
    {
        if(layoutParam.flexBasis().type() == LayoutLength::LENGTH_TYPE_AUTO)
            YGNodeStyleSetFlexBasisAuto(node);
        else if(layoutParam.flexBasis().type() == LayoutLength::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetFlexBasis(node, flexBasis.value());
        else
        {
            ASSERT(layoutParam.flexBasis().type() == LayoutLength::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetFlexBasisPercent(node, flexBasis.value());
        }
    }

    YGNodeStyleSetFlexWrap(node, toYGWrap(layoutParam.flexWrap()));
    YGNodeStyleSetJustifyContent(node, toYGJustify(layoutParam.justifyContent()));
    YGNodeStyleSetAlignItems(node, toYGAlign(layoutParam.alignItems()));
    YGNodeStyleSetAlignSelf(node, toYGAlign(layoutParam.alignSelf()));
    YGNodeStyleSetAlignContent(node, toYGAlign(layoutParam.alignContent()));

    if(const Optional<V4> margins = updateVar<V4>(tick, layoutParam.margins(), force))
    {
        YGNodeStyleSetMargin(node, YGEdgeTop, margins->x());
        YGNodeStyleSetMargin(node, YGEdgeRight, margins->y());
        YGNodeStyleSetMargin(node, YGEdgeBottom, margins->z());
        YGNodeStyleSetMargin(node, YGEdgeLeft, margins->w());
    }

    if(const Optional<V4> paddings = updateVar<V4>(tick, layoutParam.paddings(), force))
    {
        YGNodeStyleSetPadding(node, YGEdgeTop, paddings->x());
        YGNodeStyleSetPadding(node, YGEdgeRight, paddings->y());
        YGNodeStyleSetPadding(node, YGEdgeBottom, paddings->z());
        YGNodeStyleSetPadding(node, YGEdgeLeft, paddings->w());
    }

    const bool offsetDirty = layoutParam.offset() && layoutParam.offset().update(tick);
    return YGNodeIsDirty(node) || offsetDirty;
}

void updateLayoutResult(const Layout::Hierarchy& hierarchy)
{
    Layout::Node& layoutNode = hierarchy._node;
    const auto ygNode = static_cast<YGNodeRef>(layoutNode._tag);
    layoutNode.setPaddings(V4(YGNodeLayoutGetPadding(ygNode, YGEdgeTop), YGNodeLayoutGetPadding(ygNode, YGEdgeRight),
                              YGNodeLayoutGetPadding(ygNode, YGEdgeBottom), YGNodeLayoutGetPadding(ygNode, YGEdgeLeft)));
    layoutNode.setOffsetPosition(V2(YGNodeLayoutGetLeft(ygNode), YGNodeLayoutGetTop(ygNode)));
    layoutNode.setSize(V2(YGNodeLayoutGetWidth(ygNode), YGNodeLayoutGetHeight(ygNode)));
    for(const Layout::Hierarchy& i : hierarchy._child_nodes)
        updateLayoutResult(i);
}

bool doUpdate(Layout::Hierarchy& hierarchy, const uint32_t tick, const bool force)
{
    bool dirty = false;
    const Layout::Node& layoutNode = hierarchy._node;
    const YGNodeRef ygNode = static_cast<YGNodeRef>(layoutNode._tag);

    if(layoutNode._layout_param)
        dirty = updateLayoutParam(layoutNode, ygNode, tick, force);

    for(auto iter = hierarchy._child_nodes.begin(); iter != hierarchy._child_nodes.end(); )
    {
        if(Layout::Hierarchy& i = *iter; i._node->_tag)
        {
            dirty = doUpdate(i, tick, force) | dirty;
            ++ iter;
        }
        else
            iter = hierarchy._child_nodes.erase(iter);
    }

    return dirty;
}

class UpdatableYogaLayout final : public Updatable {
public:
    UpdatableYogaLayout(Layout::Hierarchy hierarchy)
        : _hierarchy(std::move(hierarchy)), _yg_node(doInflate(Global<YogaConfig>(), _hierarchy, nullptr))
    {
        doUpdate(_hierarchy, Timestamp::now(), true);
    }
    ~UpdatableYogaLayout() override
    {
        YGNodeFreeRecursive(_yg_node);
    }

    bool update(const uint32_t tick) override
    {
        const LayoutParam& layoutParam = _hierarchy._node->_layout_param;
        ASSERT(layoutParam.width().type() != LayoutLength::LENGTH_TYPE_PERCENTAGE && layoutParam.height().type() != LayoutLength::LENGTH_TYPE_PERCENTAGE);

        if(!doUpdate(_hierarchy, tick, false))
            return false;

        const float availableWidth = layoutParam.width().isAuto() ? YGUndefined : layoutParam.contentWidth();
        const float availableHeight = layoutParam.height().isAuto() ? YGUndefined : layoutParam.contentHeight();
        YGNodeCalculateLayout(_yg_node, availableWidth, availableHeight, YGDirectionLTR);
        updateLayoutResult(_hierarchy);
        return true;
    }

private:
    Layout::Hierarchy _hierarchy;
    YGNodeRef _yg_node;
};

}

sp<Updatable> YogaLayout::inflate(Hierarchy hierarchy)
{
    return sp<Updatable>::make<UpdatableYogaLayout>(std::move(hierarchy));
}

bool YogaLayout::removeNode(Node& node)
{
    const YGNodeRef ygNode = static_cast<YGNodeRef>(node._tag);
    if(const YGNodeRef ygParentNode = YGNodeGetParent(ygNode))
    {
        YGNodeRemoveChild(ygParentNode, ygNode);
        YGNodeFreeRecursive(ygNode);
        node._tag = nullptr;
        return true;
    }
    return false;
}

sp<Layout> YogaLayout::BUILDER::build(const Scope& args)
{
    return sp<Layout>::make<YogaLayout>();
}

}
