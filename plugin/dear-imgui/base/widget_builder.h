#pragma once

#include <functional>
#include <stack>

#include "core/forwarding.h"
#include "core/base/observer.h"
#include "core/base/string.h"
#include "core/inf/variable.h"
#include "core/types/shared_ptr.h"
#include "core/util/boolean_type.h"

#include "graphics/forwarding.h"
#include "graphics/base/v2.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"
#include "dear-imgui/inf/widget.h"

namespace ark::plugin::dear_imgui {

class ARK_PLUGIN_DEAR_IMGUI_API WidgetBuilder {
public:
//  [[script::bindings::auto]]
    WidgetBuilder(const sp<Renderer>& imguiRenderer);

//  [[script::bindings::auto]]
    bool begin(String name, sp<Boolean> isOpen = nullptr);
//  [[script::bindings::auto]]
    void end();

//  [[script::bindings::auto]]
    sp<Vec2> getCursorScreenPos();
//  [[script::bindings::auto]]
    sp<Vec2> getContentRegionAvail();
//  [[script::bindings::auto]]
    sp<Vec2> getItemRectMin();
//  [[script::bindings::auto]]
    sp<Vec2> getItemRectMax();

//  [[script::bindings::auto]]
    void bulletText(String content);
//  [[script::bindings::auto]]
    sp<Observer> button(const String& label, const V2& size = V2());
//  [[script::bindings::auto]]
    sp<Observer> smallButton(const String& label);

//  [[script::bindings::auto]]
    void checkbox(const String& label, const sp<Boolean>& option);
//  [[script::bindings::auto]]
    void radioButton(const String& label, const sp<Integer>& option, int32_t group);

//  [[script::bindings::auto]]
    void combo(const String& label, const sp<Integer>& option, const Vector<String>& items);
//  [[script::bindings::auto]]
    void listBox(const String& label, const sp<Integer>& option, const Vector<String>& items);

//  [[script::bindings::auto]]
    void indent(float w = 0);
//  [[script::bindings::auto]]
    void unindent(float w = 0);

//  [[script::bindings::auto]]
    sp<Widget> sameLine(float offsetFromStartX = 0.0f, float spacing = -1.0f);
//  [[script::bindings::auto]]
    sp<Widget> newLine();
//  [[script::bindings::auto]]
    void separator();
//  [[script::bindings::auto]]
    void spacing();
//  [[script::bindings::auto]]
    void text(String text);
//  [[script::bindings::auto]]
    void textWrapped(String text);

//  [[script::bindings::auto]]
    sp<Observer> inputText(String label, sp<StringVar> value, size_t maxLength = 64, int32_t flags = 0);

//  [[script::bindings::auto]]
    void inputInt(const String& label, const sp<Integer>& value, int32_t step = 1, int32_t stepFast = 100, int32_t flags = 0);
//  [[script::bindings::auto]]
    void sliderInt(const String& label, const sp<Integer>& value, int32_t vMin, int32_t vMax, const String& format = "%d");

//  [[script::bindings::auto]]
    void inputFloat(const String& label, const sp<Numeric>& v, float step = 0.0f, float step_fast = 0.0f, const String& format = "%.3f", int32_t extra_flags = 0);
//  [[script::bindings::auto]]
    void inputFloat2(const String& label, const sp<Vec2>& value, const String& format = "%.3f", int32_t flags = 0);
//  [[script::bindings::auto]]
    void inputFloat3(const String& label, const sp<Size>& value, const String& format = "%.3f", int32_t flags = 0);
//  [[script::bindings::auto]]
    void inputFloat3(const String& label, const sp<Vec3>& value, const String& format = "%.3f", int32_t flags = 0);
//  [[script::bindings::auto]]
    void inputFloat4(const String& label, const sp<Vec4>& value, const String& format = "%.3f", int32_t flags = 0);

//  [[script::bindings::auto]]
    void sliderFloat(const String& label, const sp<Numeric>& value, float vMin, float vMax, const String& format = "%.3f", float power = 1.0f);
//  [[script::bindings::auto]]
    void sliderFloat2(const String& label, const sp<Vec2>& value, float vMin, float vMax, const String& format = "%.3f", float power = 1.0f);
//  [[script::bindings::auto]]
    void sliderFloat3(const String& label, const sp<Size>& value, float vMin, float vMax, const String& format = "%.3f", float power = 1.0f);
//  [[script::bindings::auto]]
    void sliderFloat3(const String& label, const sp<Vec3>& value, float vMin, float vMax, const String& format = "%.3f", float power = 1.0f);
//  [[script::bindings::auto]]
    void sliderFloat4(const String& label, const sp<Vec4>& value, float vMin, float vMax, const String& format = "%.3f", float power = 1.0f);

//  [[script::bindings::auto]]
    void colorEdit3(const String& label, const sp<Vec3>& value);
//  [[script::bindings::auto]]
    void colorEdit4(const String& label, const sp<Color>& value);
//  [[script::bindings::auto]]
    void colorEdit4(const String& label, const sp<Vec4>& value);
//  [[script::bindings::auto]]
    void colorPicker3(const String& label, const sp<Vec3>& value);
//  [[script::bindings::auto]]
    void colorPicker4(const String& label, const sp<Color>& value);
//  [[script::bindings::auto]]
    void colorPicker4(const String& label, const sp<Vec4>& value);

//  [[script::bindings::auto]]
    void pushID(const String& id);
//  [[script::bindings::auto]]
    void pushID(int64_t id);
//  [[script::bindings::auto]]
    void popID();

//  [[script::bindings::auto]]
    void image(sp<Texture> texture, sp<Vec2> size = nullptr, const V2& uv0 = V2(0), const V2& uv1 = V2(1.0f), sp<Vec4> color = nullptr, sp<Vec4> borderColor = nullptr);

//  [[script::bindings::auto]]
    sp<Boolean> beginTabBar(String strId, int32_t flags = 0);
//  [[script::bindings::auto]]
    void endTabBar();
//  [[script::bindings::auto]]
    sp<Boolean> beginTabItem(String label, const sp<Boolean>& pOpen = nullptr, int32_t flags = 0);
//  [[script::bindings::auto]]
    void endTabItem();

//  [[script::bindings::auto]]
    sp<Widget> makeAboutWidget(sp<Boolean> isOpen = BooleanType::create(true));
//  [[script::bindings::auto]]
    sp<Widget> makeDemoWidget(sp<Boolean> isOpen = BooleanType::create(true));

//  [[script::bindings::auto]]
    void addWidget(sp<Widget> widget);

//  [[script::bindings::auto]]
    sp<Widget> makeWidget() const;

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

    template<typename T> class Invocation final : public Widget {
    public:
        typedef std::function<void(typename Argument<T>::ArgType)> FuncType;

        Invocation(FuncType func, T arg)
            : _func(std::move(func)), _arg(arg) {
        }

        void render() override {
            _func(_arg());
        }

    private:
        FuncType _func;
        Argument<T> _arg;
    };

    template<typename T> class Callback final : public Widget {
    public:
        typedef std::function<bool(typename Argument<T>::ArgType)> FuncType;

        Callback(FuncType func, T arg, sp<Runnable> callback)
            : _func(std::move(func)), _arg(std::move(arg)), _callback(std::move(callback)) {
        }

        void render() override {
            if(_func(_arg()))
                _callback->run();
        }

    private:
        FuncType _func;
        Argument<T> _arg;
        sp<Runnable> _callback;
    };

private:
    void addWidgetGroupAndPush(sp<WidgetGroup> widgetGroup);

    void addFunctionCall(std::function<void(void)> func);

    template<typename T> void addInvocation(std::function<void(typename Argument<T>::ArgType)> func, T arg) {
        addWidget(sp<Invocation<T>>::make(std::move(func), std::move(arg)));
    }

    template<typename T> void addCallback(std::function<bool(typename Argument<T>::ArgType)> func, T arg, sp<Observer> observer) {
        addWidget(sp<Callback<T>>::make(std::move(func), std::move(arg), std::move(observer)));
    }

    void push(sp<WidgetGroup> widgetGroup);
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
