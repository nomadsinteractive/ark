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
    class STYLE_DISPLAY : public Builder<Renderer> {
    public:
        STYLE_DISPLAY(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;

        LayoutParam::Display _display;
    };

//  [[plugin::style("gravity")]]
    class STYLE_GRAVITY : public Builder<Renderer> {
    public:
        STYLE_GRAVITY(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;

        LayoutParam::Gravity _gravity;
    };

//  [[plugin::style("size")]]
    class STYLE_SIZE : public Builder<Renderer> {
    public:
        STYLE_SIZE(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        sp<Builder<Renderer>> _delegate;
        sp<Builder<Size>> _size;
    };

//  [[plugin::style("margins")]]
    class STYLE_MARGINS : public Builder<Renderer> {
    public:
        STYLE_MARGINS(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margins;

    };

//  [[plugin::style("margin-left")]]
    class STYLE_MARGIN_LEFT : public Builder<Renderer> {
    public:
        STYLE_MARGIN_LEFT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_left;

    };

//  [[plugin::style("margin-right")]]
    class STYLE_MARGIN_RIGHT : public Builder<Renderer> {
    public:
        STYLE_MARGIN_RIGHT(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_right;
    };

//  [[plugin::style("margin-top")]]
    class STYLE_MARGIN_TOP : public Builder<Renderer> {
    public:
        STYLE_MARGIN_TOP(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_top;
    };

//  [[plugin::style("margin-bottom")]]
    class STYLE_MARGIN_BOTTOM : public Builder<Renderer> {
    public:
        STYLE_MARGIN_BOTTOM(BeanFactory& beanFactory, const sp<Builder<Renderer>>& delegate, const String& style);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BeanFactory _bean_factory;
        sp<Builder<Renderer>> _delegate;
        String _margin_bottom;
    };

//  [[plugin::style("stop-propagation")]]
    class STOP_PROPAGATION_PARAM : public Builder<Renderer> {
    public:
        STOP_PROPAGATION_PARAM(BeanFactory& factory, const sp<Builder<Renderer>>& delegate, const String& style);

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
