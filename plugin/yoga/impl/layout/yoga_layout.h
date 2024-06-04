#pragma once

#include <yoga/YGNode.h>
#include <yoga/Yoga.h>

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "app/inf/layout.h"

namespace ark::plugin::yoga {

class YogaLayout : public Layout {
public:
    YogaLayout();
    virtual ~YogaLayout() override;

    virtual void inflate(sp<Node> rootNode) override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("yoga")]]
    class BUILDER : public Builder<Layout> {
    public:
        BUILDER(BeanFactory& factory);

        virtual sp<Layout> build(const Scope& args) override;
    };

private:
    class YogaConfig {
    public:
        YogaConfig();
        ~YogaConfig();

        YGNodeRef newNode() const;

    private:
        YGConfigRef _config;
    };

private:
    static void applyLayoutParam(const LayoutParam& layoutParam, YGNodeRef node, uint64_t timestamp);
    static void updateLayoutResult(Layout::Node& layoutNode);

    static YGNodeRef doInflate(const YogaConfig& config, Layout::Node& layoutNode, YGNodeRef parentNode);
    static void doUpdate(Layout::Node& layoutNode, uint64_t timestamp);

private:
    YGNodeRef _yg_node;
    sp<Node> _root_node;
};

}
