#pragma once

#include <stack>

#include "core/base/api.h"
#include "core/types/shared_ptr.h"

#include "app/base/event.h"
#include "app/inf/event_listener.h"

namespace ark {

class ARK_API EventDispatcher final : public EventListener {
public:
//  [[script::bindings::auto]]
    EventDispatcher();

//  [[script::bindings::property]]
    bool locked() const;
//  [[script::bindings::property]]
    void setLocked(bool locked);

//  [[script::bindings::auto]]
    void onKeyEvent(Event::Code code, sp<Runnable> onPress = nullptr, sp<Runnable> onRelease = nullptr, sp<Runnable> onRepeat = nullptr);
//  [[script::bindings::auto]]
    void unKeyEvent(Event::Code code);

//  [[script::bindings::auto]]
    void onMotionEvent(sp<EventListener> onPress = nullptr, sp<EventListener> onRelease = nullptr, sp<EventListener> onClick = nullptr, sp<EventListener> onMove = nullptr);
//  [[script::bindings::auto]]
    void unMotionEvent();

    float motionClickRange() const;

//  [[script::bindings::auto]]
    bool onEvent(const Event& event) override;

private:
    class KeyEventListener final : public EventListener {
    public:
        KeyEventListener(sp<Runnable> onPress, sp<Runnable> onRelease, sp<Runnable> onRepeat);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(KeyEventListener);

        bool onEvent(const Event& event) override;

    private:
        sp<Runnable> _on_press;
        sp<Runnable> _on_release;
        sp<Runnable> _on_repeat;
        uint64_t _on_press_timestamp;
    };

    class MotionEventListener {
    public:
        MotionEventListener(sp<EventListener> onPress, sp<EventListener> onRelease, sp<EventListener> onClick, sp<EventListener> onMove);
        DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(MotionEventListener);

        bool onEvent(const EventDispatcher& dispatcher, const Event& event);

    private:
        sp<EventListener> _on_press;
        sp<EventListener> _on_release;
        sp<EventListener> _on_click;
        sp<EventListener> _on_move;

        float _pressed_x;
        float _pressed_y;
    };

private:
    float _motion_click_range;
    bool _locked;

    Map<Event::Code, std::stack<KeyEventListener>> _key_events;
    std::stack<MotionEventListener> _motion_events;
};

}
