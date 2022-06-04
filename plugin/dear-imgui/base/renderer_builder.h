#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_BUILDER_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_BUILDER_H_

#include <functional>
#include <stack>
#include <vector>

#include "core/forwarding.h"
#include "core/base/observer.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"
#include "dear-imgui/inf/widget.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class WidgetGroup;

class ARK_PLUGIN_DEAR_IMGUI_API RendererBuilder {
public:
// [[script::bindings::auto]]
    RendererBuilder(const sp<Renderer>& imguiRenderer);

// [[script::bindings::auto]]
    bool begin(const String& name);
// [[script::bindings::auto]]
    void end();

// [[script::bindings::auto]]
    void bulletText(const String& name);
// [[script::bindings::auto]]
    sp<Observer> button(const String& label, const V2& size = V2());
// [[script::bindings::auto]]
    sp<Observer> smallButton(const String& label);

// [[script::bindings::auto]]
    void checkbox(const String& label, const sp<Boolean>& option);
// [[script::bindings::auto]]
    void radioButton(const String& label, const sp<Integer>& option, int32_t group);

// [[script::bindings::auto]]
    void combo(const String& label, const sp<Integer>& option, const std::vector<String>& items);
// [[script::bindings::auto]]
    void listBox(const String& label, const sp<Integer>& option, const std::vector<String>& items);

// [[script::bindings::auto]]
    void indent(float w = 0);
// [[script::bindings::auto]]
    void unindent(float w = 0);

// [[script::bindings::auto]]
    void sameLine(float localPosX = 0.0f, float spacingW = -1.0f);
// [[script::bindings::auto]]
    void separator();
// [[script::bindings::auto]]
    void spacing();
// [[script::bindings::auto]]
    void text(const String& name);

// [[script::bindings::auto]]
    sp<Observer> inputText(String label, sp<Text::Impl> value, size_t maxLength = 64, int32_t flags = 0);

// [[script::bindings::auto]]
    void inputInt(const String& label, const sp<Integer>& value, int32_t step = 1, int32_t step_fast = 100, int32_t flags = 0);

// [[script::bindings::auto]]
    void inputFloat(const String& label, const sp<Numeric>& value, float step = 0.0f, float step_fast = 0.0f, const String& format = "%.3f");
// [[script::bindings::auto]]
    void inputFloat2(const String& label, const sp<Vec2>& value, const String& format = "%.3f", int32_t flags = 0);
// [[script::bindings::auto]]
    void inputFloat3(const String& label, const sp<Size>& size, const String& format = "%.3f", int32_t flags = 0);
// [[script::bindings::auto]]
    void inputFloat3(const String& label, const sp<Vec3>& value, const String& format = "%.3f", int32_t flags = 0);
// [[script::bindings::auto]]
    void inputFloat4(const String& label, const sp<Vec4>& value, const String& format = "%.3f", int32_t flags = 0);

// [[script::bindings::auto]]
    void sliderFloat(const String& label, const sp<Numeric>& value, float v_min, float v_max, const String& format = "%.3f", float power = 1.0f);
// [[script::bindings::auto]]
    void sliderFloat2(const String& label, const sp<Vec2>& value, float v_min, float v_max, const String& format = "%.3f", float power = 1.0f);
// [[script::bindings::auto]]
    void sliderFloat3(const String& label, const sp<Size>& size, float v_min, float v_max, const String& format = "%.3f", float power = 1.0f);
// [[script::bindings::auto]]
    void sliderFloat3(const String& label, const sp<Vec3>& value, float v_min, float v_max, const String& format = "%.3f", float power = 1.0f);
// [[script::bindings::auto]]
    void sliderFloat4(const String& label, const sp<Vec4>& value, float v_min, float v_max, const String& format = "%.3f", float power = 1.0f);

// [[script::bindings::auto]]
    void colorEdit3(const String& label, const sp<Vec3>& value);
// [[script::bindings::auto]]
    void colorEdit4(const String& label, const sp<Vec4>& value);
// [[script::bindings::auto]]
    void colorPicker3(const String& label, const sp<Vec3>& value);
// [[script::bindings::auto]]
    void colorPicker4(const String& label, const sp<Vec4>& value);

// [[script::bindings::auto]]
    void image(const sp<Texture>& texture, const sp<Vec2>& size, const V2& uv0 = V2(0), const V2& uv1 = V2(1.0f), const sp<Vec4>& color = nullptr, const sp<Vec4>& borderColor = nullptr);

// [[script::bindings::auto]]
    sp<Controller> showAboutWindow();
// [[script::bindings::auto]]
    sp<Controller> showDemoWindow();

// [[script::bindings::auto]]
    sp<Renderer> build() const;

private:

    template<typename T> struct Argument {
        typedef T   ArgType;

        Argument(T arg)
            : _arg(arg) {
        }

        T operator()() const {
            return _arg;
        }

        T _arg;
    };

    template<> struct Argument<String> {
        typedef const char*   ArgType;

        Argument(String arg)
            : _arg(std::move(arg)) {
        }

        ArgType operator()() const {
            return _arg.c_str();
        }

        String _arg;
    };

    template<typename T> class Invocation : public Widget {
    public:
        typedef std::function<void(typename Argument<T>::ArgType)> FuncType;

        Invocation(FuncType func, T arg)
            : _func(std::move(func)), _arg(arg) {
        }

        virtual void render() override {
            _func(_arg());
        }

    private:
        FuncType _func;
        Argument<T> _arg;
    };

    template<typename T> class Callback : public Widget {
    public:
        typedef std::function<bool(typename Argument<T>::ArgType)> FuncType;

        Callback(FuncType func, T arg, const sp<Observer>& observer)
            : _func(std::move(func)), _arg(std::move(arg)), _observer(observer) {
        }

        virtual void render() override {
            if(_func(_arg()))
                _observer->update();
        }

    private:
        FuncType _func;
        Argument<T> _arg;
        sp<Observer> _observer;
    };

private:
    void addBlock(std::function<void(void)> func);

    template<typename T> void addInvocation(std::function<void(typename Argument<T>::ArgType)> func, T arg) {
        addWidget(sp<Invocation<T>>::make(std::move(func), std::move(arg)));
    }

    template<typename T> void addCallback(std::function<bool(typename Argument<T>::ArgType)> func, T arg, const sp<Observer>& observer) {
        addWidget(sp<Callback<T>>::make(std::move(func), std::move(arg), observer));
    }

    void addWidget(const sp<Widget>& widget);

    void push(const sp<WidgetGroup>& widget);
    void pop();

    const sp<WidgetGroup>& current() const;

private:
    struct Stub {
        std::stack<sp<WidgetGroup>> _states;
    };

private:
    sp<RendererContext> _renderer_context;

    sp<Stub> _stub;
};


}
}
}

#endif
