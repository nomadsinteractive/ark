#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/wirable.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/components/text.h"
#include "graphics/inf/layout.h"
#include "graphics/inf/renderable.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API View final : public Wirable {
public:
    View(sp<LayoutParam> layoutParam, String name, sp<RenderObject> background = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);
    ~View() override;

    TypeId onPoll(WiringContext& context) override;
    void onWire(const WiringContext& context) override;

    bool updateLayout(uint64_t timestamp) const;

    const sp<Layout::Node>& layoutNode() const;

//  [[script::bindings::property]]
    const SafeVar<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisbile(sp<Boolean> visible);

//  [[script::bindings::property]]
    const SafeVar<Boolean>& discarded() const;
//  [[script::bindings::property]]
    void setDiscarded(sp<Boolean> discarded);

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::property]]
    const sp<Vec3>& layoutPosition();
//  [[script::bindings::property]]
    const sp<Size>& layoutSize();

//  [[script::bindings::auto]]
    void addView(sp<View> view, sp<Boolean> discarded = nullptr);
//  [[script::bindings::auto]]
    sp<View> findView(StringView name) const;

    sp<Boundaries> makeBoundaries();

    const sp<ViewHierarchy>& hierarchy() const;

    void setParent(const View& view);

//  [[plugin::builder]]
    class BUILDER final : public Builder<View> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<View> build(const Scope& args) override;

    private:
        String _name;
        SafeBuilder<Boolean> _discarded;
        SafeBuilder<Boolean> _visible;
        SafeBuilder<RenderObject> _background;
        SafeBuilder<LayoutParam> _layout_param;
        std::vector<builder<View>> _children;
    };

//  [[plugin::builder("with-view")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        builder<View> _view;
    };

//  [[plugin::builder("text")]]
    class BUILDER_VIEW final : public Builder<View> {
    public:
        BUILDER_VIEW(BeanFactory& factory, const document& manifest);

        sp<View> build(const Scope& args) override;

    private:
        BUILDER _builder_impl;
        Text::BUILDER _builder_text;
    };

    struct Stub;

private:
    void markAsTopView();

protected:
    sp<Stub> _stub;
    sp<RenderObject> _background;
    sp<Boolean> _is_discarded;
    sp<Updatable> _updatable_view;
    sp<Updatable> _updatable_layout;

    sp<Vec3> _layout_position;
    sp<Size> _layout_size;

    friend class Activity;
    friend class ViewHierarchy;
};

}
