#include "yoga/impl/layout/yoga_layout.h"

#include <yoga/YGNode.h>
#include <yoga/Yoga.h>

#include "core/ark.h"
#include "core/types/global.h"
#include "core/base/bean_factory.h"

#include "graphics/base/v2.h"
#include "graphics/traits/layout_param.h"

#include "app/view/view.h"
#include "app/view/view_hierarchy.h"

namespace ark::plugin::yoga {

namespace {

YGFlexDirection toYGFlexDirection(LayoutParam::FlexDirection flexDirection)
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

YGWrap toYGWrap(LayoutParam::FlexWrap flexWrap)
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

YGJustify toYGJustify(LayoutParam::JustifyContent justifyContent)
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

YGAlign toYGAlign(LayoutParam::Align align)
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

template<typename T, typename U> Optional<T> updateVar(uint64_t timestamp, U& var)
{
    if(!timestamp || var.update(timestamp))
        return var.val();
    return Optional<T>();
}

void applyLayoutParam(const LayoutParam& layoutParam, YGNodeRef node, uint64_t timestamp)
{
    if(const Optional<float> width = updateVar<float>(timestamp, layoutParam.width()._value))
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

    if(const Optional<float> height = updateVar<float>(timestamp, layoutParam.height()._value))
    {
        if(layoutParam.height()._type == LayoutParam::LENGTH_TYPE_AUTO)
            YGNodeStyleSetHeightAuto(node);
        else if(layoutParam.height()._type == LayoutParam::LENGTH_TYPE_PIXEL)
            YGNodeStyleSetHeight(node, height.value());
        else
        {
            ASSERT(layoutParam.height()._type == LayoutParam::LENGTH_TYPE_PERCENTAGE);
            YGNodeStyleSetHeightPercent(node, height.value());
        }
    }

    YGNodeStyleSetFlexDirection(node, toYGFlexDirection(layoutParam.flexDirection()));
    YGNodeStyleSetFlexGrow(node, layoutParam.flexGrow());

    if(const Optional<float> flexBasis = updateVar<float, const SafeVar<Numeric>>(timestamp, layoutParam.flexBasis()))
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

void doUpdate(const Layout::Hierarchy& hierarchy, uint64_t timestamp)
{
    Layout::Node& layoutNode = hierarchy._node;
    YGNodeRef ygNode = static_cast<YGNodeRef>(layoutNode._tag);

    if(layoutNode._layout_param)
        applyLayoutParam(layoutNode._layout_param, ygNode, timestamp);

    for(const Layout::Hierarchy& i : hierarchy._child_nodes)
        doUpdate(i, timestamp);
}

class UpdatableYogaLayout : public Updatable {
public:
    UpdatableYogaLayout(Layout::Hierarchy hierarchy)
        : _hierarchy(std::move(hierarchy)), _yg_node(doInflate(Global<YogaConfig>(), _hierarchy, nullptr)) {
    }
    ~UpdatableYogaLayout() override {
        YGNodeFreeRecursive(_yg_node);
    }

    bool update(uint64_t timestamp) override {
        const LayoutParam& layoutParam = _hierarchy._node->_layout_param;
        ASSERT(layoutParam.widthType() != LayoutParam::LENGTH_TYPE_PERCENTAGE && layoutParam.height()._type != LayoutParam::LENGTH_TYPE_PERCENTAGE);
        doUpdate(_hierarchy, timestamp);
        YGNodeCalculateLayout(_yg_node, layoutParam.contentWidth(), layoutParam.contentHeight(), YGDirectionLTR);
        updateLayoutResult(_hierarchy);
        return YGNodeIsDirty(_yg_node);
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

YogaLayout::BUILDER::BUILDER(BeanFactory& factory)
{
}

sp<Layout> YogaLayout::BUILDER::build(const Scope& args)
{
    return sp<YogaLayout>::make();
}

}
