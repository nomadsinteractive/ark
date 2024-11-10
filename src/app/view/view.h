#pragma once

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/builder.h"
#include "core/inf/updatable.h"
#include "core/inf/wirable.h"
#include "core/types/safe_var.h"

#include "graphics/forwarding.h"
#include "graphics/base/text.h"
#include "graphics/inf/layout.h"
#include "graphics/inf/renderable.h"

#include "app/forwarding.h"

namespace ark {

class ARK_API View final : public Wirable {
public:
    View(sp<LayoutParam> layoutParam, sp<RenderObject> background = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> discarded = nullptr);
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

//  [[script::bindings::auto]]
    void addView(sp<View> view, sp<Boolean> discarded = nullptr);

    const sp<ViewHierarchy>& hierarchy() const;

    void setParent(const View& view);

//  [[plugin::builder]]
    class BUILDER final : public Builder<View> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        sp<View> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        SafeBuilder<Boolean> _discarded;
        SafeBuilder<Boolean> _visible;
        SafeBuilder<RenderObject> _background;
        SafeBuilder<LayoutParam> _layout_param;
    };

//  [[plugin::builder("view")]]
    class BUILDER_WIRABLE final : public Builder<Wirable> {
    public:
        BUILDER_WIRABLE(BeanFactory& factory, const document& manifest);

        sp<Wirable> build(const Scope& args) override;

    private:
        BUILDER _builder_impl;
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

    friend class Arena;
    friend class ViewHierarchy;
};

}
