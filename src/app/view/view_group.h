#ifndef ARK_APP_VIEW_VIEW_GROUP_H_
#define ARK_APP_VIEW_VIEW_GROUP_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/types/implements.h"
#include "core/types/safe_ptr.h"

#include "graphics/inf/renderer.h"
#include "graphics/base/frame.h"

#include "app/forwarding.h"
#include "app/inf/event_listener.h"
#include "app/view/view.h"

namespace ark {

class ARK_API ViewGroup final : public View, public Renderer, public Renderer::Group, Implements<ViewGroup, View, Block, Renderer, Renderer::Group, Holder> {
public:
    ViewGroup(const Frame& background, const sp<Layout>& layout, const sp<LayoutParam>& layoutParam);
    ~ViewGroup() override;

    virtual void addRenderer(const sp<Renderer>& renderer) override;
    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual void traverse(const Visitor& visitor) override;

    bool onEvent(const Event& event, float x, float y);

//  [[plugin::builder("view-group")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        SafePtr<Builder<Layout>> _layout;
        SafePtr<Builder<Renderer>> _background;
        sp<Builder<LayoutParam>> _layout_param;
    };

private:
    sp<Renderer> _background;

    sp<LayoutHierarchy> _layout_hierarchy;

};

}

#endif
