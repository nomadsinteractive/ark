#ifndef ARK_APP_VIEW_VIEW_H_
#define ARK_APP_VIEW_VIEW_H_

#include "core/base/api.h"
#include "core/base/bean_factory.h"
#include "core/inf/holder.h"

#include "graphics/base/rect.h"
#include "graphics/inf/block.h"
#include "graphics/forwarding.h"

#include "app/forwarding.h"
#include "app/view/layout_param.h"

namespace ark {

class ARK_API View : public Block, public Holder {
public:
    enum State {
        STATE_DEFAULT = 0,
        STATE_PUSHING = 1,
        STATE_MOVING = 2,
        STATE_ACTIVED = 4,
        STATE_COUNT = 4
    };

    enum Gravity {
        NONE = 0,
        LEFT = 1,
        RIGHT = 2,
        CENTER_HORIZONTAL = 3,
        TOP = 4,
        BOTTOM = 8,
        CENTER_VERTICAL = 12,
        CENTER = CENTER_VERTICAL | CENTER_HORIZONTAL,
        GRAVITY_DEFAULT = CENTER
    };

public:
    View(const sp<LayoutParam>& layoutParam);
    View(const sp<Size>& size);

    virtual const SafePtr<Size>& size() override;

    virtual void traverse(const Visitor& visitor) override;

    const SafePtr<LayoutParam>& layoutParam() const;
    void setLayoutParam(const sp<LayoutParam>& layoutParam);

    State state() const;
    void addState(State state);
    void removeState(State state);

    const sp<Runnable>& onEnter() const;
    void setOnEnter(const sp<Runnable>& fireOnEnter);
    const sp<Runnable>& onLeave() const;
    void setOnLeave(const sp<Runnable>& fireOnLeave);
    const sp<Runnable>& onPush() const;
    void setOnPush(const sp<Runnable>& fireOnPush);
    const sp<Runnable>& onClick() const;
    void setOnClick(const sp<Runnable>& fireOnClick);
    const sp<Runnable>& onRelease() const;
    void setOnRelease(const sp<Runnable>& fireOnRelease);
    const sp<EventListener>& onMove() const;
    void setOnMove(const sp<EventListener>& fireOnMove);

    bool dispatchEvent(const Event& event, bool ptin);

//  [[plugin::style("display")]]
    class DISPLAY_DECORATOR : public Builder<Renderer> {
    public:
        DISPLAY_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;

        LayoutParam::Display _display;
    };

//  [[plugin::style("size")]]
    class SIZE_DECORATOR : public Builder<Renderer> {
    public:
        SIZE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Size>> _size;
    };

//  [[plugin::style("margins")]]
    class MARGINS_DECORATOR : public Builder<Renderer> {
    public:
        MARGINS_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margins;

    };

//  [[plugin::style("margin-left")]]
    class MARGIN_LEFT_DECORATOR : public Builder<Renderer> {
    public:
        MARGIN_LEFT_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_left;

    };

//  [[plugin::style("margin-right")]]
    class MARGIN_RIGHT_DECORATOR : public Builder<Renderer> {
    public:
        MARGIN_RIGHT_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_right;
    };

//  [[plugin::style("margin-top")]]
    class MARGIN_TOP_DECORATOR : public Builder<Renderer> {
    public:
        MARGIN_TOP_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_top;
    };

//  [[plugin::style("margin-bottom")]]
    class MARGIN_BOTTOM_DECORATOR : public Builder<Renderer> {
    public:
        MARGIN_BOTTOM_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_bottom;
    };

//  [[plugin::style("layout-param")]]
    class LAYOUT_PARAM_DECORATOR : public Builder<Renderer> {
    public:
        LAYOUT_PARAM_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<LayoutParam>> _layout_param;
    };

//  [[plugin::style("onenter")]]
    class ON_ENTER_DECORATOR : public Builder<Renderer> {
    public:
        ON_ENTER_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_enter;
    };

//  [[plugin::style("onleave")]]
    class ON_LEAVE_DECORATOR : public Builder<Renderer> {
    public:
        ON_LEAVE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_leave;
    };

//  [[plugin::style("onpush")]]
    class ON_PUSH_DECORATOR : public Builder<Renderer> {
    public:
        ON_PUSH_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_push;
    };

//  [[plugin::style("onclick")]]
    class ON_CLICK_DECORATOR : public Builder<Renderer> {
    public:
        ON_CLICK_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_click;

    };

//  [[plugin::style("onrelease")]]
    class ON_RELEASE_DECORATOR : public Builder<Renderer> {
    public:
        ON_RELEASE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Runnable>> _on_release;

    };

//  [[plugin::style("onmove")]]
    class ON_MOVE_DECORATOR : public Builder<Renderer> {
    public:
        ON_MOVE_DECORATOR(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<EventListener>> _on_move;

    };

protected:
    virtual bool fireOnEnter();
    virtual bool fireOnLeave();
    virtual bool fireOnPush();
    virtual bool fireOnRelease();
    virtual bool fireOnClick();

    virtual bool fireOnMove(const Event& event);

protected:
    SafePtr<LayoutParam> _layout_param;

    sp<State> _state;
    sp<Runnable> _on_enter;
    sp<Runnable> _on_leave;
    sp<Runnable> _on_push;
    sp<Runnable> _on_click;
    sp<Runnable> _on_release;

    sp<EventListener> _on_move;

};

}

#endif
