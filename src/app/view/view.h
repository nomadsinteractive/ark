#ifndef ARK_APP_VIEW_VIEW_H_
#define ARK_APP_VIEW_VIEW_H_

#include <vector>

#include "core/forwarding.h"
#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/impl/updatable/updatable_wrapper.h"
#include "core/inf/builder.h"
#include "core/inf/holder.h"
#include "core/inf/updatable.h"
#include "core/types/implements.h"
#include "core/types/safe_ptr.h"
#include "core/types/safe_var.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/inf/renderable.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/inf/layout_event_listener.h"
#include "app/inf/layout.h"

namespace ark {

//[[script::bindings::extends(Renderer)]]
class ARK_API View : public Block, public Renderer, public Renderer::Group, public LayoutEventListener, public Holder,
                     Implements<View, Block, Renderer, Renderer::Group, LayoutEventListener, Holder> {
public:
    enum State {
        STATE_DEFAULT = 0,
        STATE_PUSHING = 1,
        STATE_MOVING = 2,
        STATE_ACTIVED = 4,
        STATE_COUNT = 4
    };

public:
    View(const sp<LayoutParam>& layoutParam, sp<RenderObjectWithLayer> background = nullptr, sp<Text> text = nullptr, sp<Layout> layout = nullptr, sp<LayoutV3> layoutV3 = nullptr, sp<Boolean> visible = nullptr, sp<Boolean> disposed = nullptr);
    View(sp<Size> size);
    ~View() override;

//  [[script::bindings::property]]
    virtual const sp<Size>& size() override;

    virtual void traverse(const Visitor& visitor) override;

    virtual void render(RenderRequest& renderRequest, const V3& position) override;
    virtual void addRenderer(const sp<Renderer>& renderer) override;

    virtual bool onEvent(const Event& event, float x, float y, bool ptin) override;

    void addRenderObjectWithLayer(sp<RenderObjectWithLayer> ro, bool isBackground);

//  [[script::bindings::property]]
    const sp<Vec3>& position() const;

    void updateLayout();
    void updateTextLayout(uint64_t timestamp);

    const sp<LayoutV3::Node>& layoutNode() const;

//  [[script::bindings::property]]
    const sp<Boolean>& visible() const;
//  [[script::bindings::property]]
    void setVisbile(sp<Boolean> visible);

//  [[script::bindings::property]]
    sp<Boolean> disposed() const;
//  [[script::bindings::property]]
    void setDisposed(sp<Boolean> disposed);

//  [[script::bindings::property]]
    const sp<LayoutParam>& layoutParam() const;
//  [[script::bindings::property]]
    void setLayoutParam(sp<LayoutParam> layoutParam);

//  [[script::bindings::auto]]
    void addView(sp<View> view);

    void setParent(const View& view);

    State state() const;
    void addState(State state);
    void removeState(State state);

//  [[script::bindings::property]]
    const sp<Runnable>& onEnter() const;
//  [[script::bindings::property]]
    void setOnEnter(const sp<Runnable>& fireOnEnter);
//  [[script::bindings::property]]
    const sp<Runnable>& onLeave() const;
//  [[script::bindings::property]]
    void setOnLeave(const sp<Runnable>& fireOnLeave);
//  [[script::bindings::property]]
    const sp<Runnable>& onPush() const;
//  [[script::bindings::property]]
    void setOnPush(const sp<Runnable>& fireOnPush);
//  [[script::bindings::property]]
    const sp<Runnable>& onClick() const;
//  [[script::bindings::property]]
    void setOnClick(const sp<Runnable>& fireOnClick);
//  [[script::bindings::property]]
    const sp<Runnable>& onRelease() const;
//  [[script::bindings::property]]
    void setOnRelease(const sp<Runnable>& fireOnRelease);

//  [[script::bindings::property]]
    const sp<EventListener>& onMove() const;
//  [[script::bindings::property]]
    void setOnMove(const sp<EventListener>& fireOnMove);

    bool dispatchEvent(const Event& event, bool ptin);

//  [[plugin::builder]]
    class BUILDER : public Builder<View> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<View> build(const Scope& args) override;

    private:
        BeanFactory _factory;
        document _manifest;

        SafePtr<Builder<Boolean>> _disposed;
        SafePtr<Builder<Boolean>> _visible;
        SafePtr<Builder<Layout>> _layout;
        SafePtr<Builder<LayoutV3>> _layout_v3;
        SafePtr<Builder<RenderObjectWithLayer>> _background;
        SafePtr<Builder<Text>> _text;
        sp<Builder<LayoutParam>> _layout_param;
    };

//  [[plugin::builder("view")]]
    class BUILDER_VIEW : public Builder<Renderer> {
    public:
        BUILDER_VIEW(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

//  [[plugin::style("layout-weight")]]
    class STYLE_LAYOUT_WEIGHT : public Builder<Renderer> {
    public:
        STYLE_LAYOUT_WEIGHT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Numeric>> _layout_weight;
    };

//  [[plugin::style("margins")]]
    class STYLE_MARGINS : public Builder<Renderer> {
    public:
        STYLE_MARGINS(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Vec4>> _margins;
    };

//  [[plugin::style("margin-left")]]
    class STYLE_MARGIN_LEFT : public Builder<Renderer> {
    public:
        STYLE_MARGIN_LEFT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Numeric>> _margin_left;

    };

//  [[plugin::style("margin-right")]]
    class STYLE_MARGIN_RIGHT : public Builder<Renderer> {
    public:
        STYLE_MARGIN_RIGHT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Numeric>> _margin_right;
    };

//  [[plugin::style("margin-top")]]
    class STYLE_MARGIN_TOP : public Builder<Renderer> {
    public:
        STYLE_MARGIN_TOP(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Numeric>> _margin_top;
    };

//  [[plugin::style("margin-bottom")]]
    class STYLE_MARGIN_BOTTOM : public Builder<Renderer> {
    public:
        STYLE_MARGIN_BOTTOM(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Numeric>> _margin_bottom;
    };

//  [[plugin::style("stop-propagation")]]
    class STOP_PROPAGATION_STYLE : public Builder<Renderer> {
    public:
        STOP_PROPAGATION_STYLE(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Boolean>> _stop_propagation;
    };

//  [[plugin::style("layout-param")]]
    class STYLE_LAYOUT_PARAM : public Builder<Renderer> {
    public:
        STYLE_LAYOUT_PARAM(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<LayoutParam>> _layout_param;
    };

//  [[plugin::style("onenter")]]
    class STYLE_ON_ENTER : public Builder<Renderer> {
    public:
        STYLE_ON_ENTER(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_enter;
    };

//  [[plugin::style("onleave")]]
    class STYLE_ON_LEAVE : public Builder<Renderer> {
    public:
        STYLE_ON_LEAVE(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_leave;
    };

//  [[plugin::style("onpush")]]
    class STYLE_ON_PUSH : public Builder<Renderer> {
    public:
        STYLE_ON_PUSH(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_push;
    };

//  [[plugin::style("onclick")]]
    class STYLE_ON_CLICK : public Builder<Renderer> {
    public:
        STYLE_ON_CLICK(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_click;

    };

//  [[plugin::style("onrelease")]]
    class STYLE_ON_RELEASE : public Builder<Renderer> {
    public:
        STYLE_ON_RELEASE(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_release;

    };

//  [[plugin::style("onmove")]]
    class STYLE_ON_MOVE : public Builder<Renderer> {
    public:
        STYLE_ON_MOVE(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<EventListener>> _on_move;

    };

    struct Stub : public Updatable {
        Stub(const sp<LayoutParam>& layoutParam, sp<ViewHierarchy> viewHierarchy, sp<Boolean> visible, sp<Boolean> disposed);

        virtual bool update(uint64_t timestamp) override;

        void dispose();

        bool isVisible() const;
        bool isDisposed() const;

        V2 getTopViewOffsetPosition() const;
        sp<LayoutV3::Node> getTopViewLayoutNode() const;

        const sp<ViewHierarchy>& viewHierarchy() const;
        ViewHierarchy& ensureViewHierarchy();

        sp<LayoutParam> _layout_param;
        sp<LayoutV3::Node> _layout_node;

        SafeVar<Boolean> _visible;
        sp<BooleanWrapper> _disposed;

        sp<Stub> _parent_stub;
        bool _top_view;
    };

    class UpdatableLayoutView : public Updatable {
    public:
        UpdatableLayoutView(sp<Stub> stub);

        virtual bool update(uint64_t timestamp) override;

    private:
        sp<Stub> _stub;
    };

private:
    bool fireOnEnter();
    bool fireOnLeave();
    bool fireOnPush();
    bool fireOnRelease();
    bool fireOnClick();

    bool fireOnMove(const Event& event);

    void markAsTopView();

    class RenderableViewSlot : public Renderable {
    public:
        RenderableViewSlot(sp<Stub> viewStub, sp<Renderable> renderable, bool isBackground);

        virtual StateBits updateState(const RenderRequest& renderRequest) override;
        virtual Snapshot snapshot(const PipelineInput& pipelineInput, const RenderRequest& renderRequest, const V3& postTranslate, StateBits state) override;

    private:
        sp<Stub> _view_stub;
        sp<Renderable> _renderable;
        bool _is_background;
    };

    class IsDisposed : public Boolean {
    public:
        IsDisposed(sp<Stub> stub);

        virtual bool update(uint64_t timestamp) override;
        virtual bool val() override;

    private:
        sp<Stub> _stub;
    };

    class LayoutPosition : public Vec3 {
    public:
        LayoutPosition(sp<Stub> stub, sp<Updatable> updatable, bool isBackground, bool isCenter);

        virtual bool update(uint64_t timestamp) override;
        virtual V3 val() override;

    private:
        sp<Stub> _stub;
        sp<Updatable> _updatable;

        bool _is_background;
        bool _is_center;
    };

protected:
    sp<Stub> _stub;
    sp<RenderObjectWithLayer> _background;
    sp<Text> _text;

    sp<State> _state;
    sp<Runnable> _on_enter;
    sp<Runnable> _on_leave;
    sp<Runnable> _on_push;
    sp<Runnable> _on_click;
    sp<Runnable> _on_release;

    sp<EventListener> _on_move;
    sp<IsDisposed> _is_disposed;
    sp<Updatable> _is_dirty;

    sp<Size> _size;
    sp<Vec3> _position;

    friend class Arena;
    friend class ViewHierarchy;
};

}

#endif
