#ifndef ARK_PLUGIN_YOGA_IMPL_LAYOUT_YOGA_LAYOUT_H_
#define ARK_PLUGIN_YOGA_IMPL_LAYOUT_YOGA_LAYOUT_H_

#include <yoga/YGNode.h>
#include <yoga/Yoga.h>

#include "core/forwarding.h"
#include "core/inf/builder.h"
#include "core/types/shared_ptr.h"

#include "app/inf/layout.h"

namespace ark {
namespace plugin {
namespace yoga {

class YogaLayout : public LayoutV3 {
public:
    YogaLayout();
    virtual ~YogaLayout() override;

    virtual void inflate(sp<Node> rootNode) override;
    virtual bool update(uint64_t timestamp) override;

//  [[plugin::builder::by-value("yoga")]]
    class BUILDER : public Builder<LayoutV3> {
    public:
        BUILDER(BeanFactory& factory);

        virtual sp<LayoutV3> build(const Scope& args) override;
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
    static void updateLayoutResult(LayoutV3::Node& layoutNode);

    static YGNodeRef doInflate(const Global<YogaConfig>& config, LayoutV3::Node& layoutNode, YGNodeRef parentNode);
    static void doUpdate(LayoutV3::Node& layoutNode, uint64_t timestamp);

private:
    YGNodeRef _yg_node;
    sp<Node> _layout_node;
};

}
}
}

#endif
