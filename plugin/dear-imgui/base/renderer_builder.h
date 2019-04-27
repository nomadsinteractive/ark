#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_BUILDER_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_BUILDER_H_

#include <functional>
#include <stack>

#include "core/forwarding.h"
#include "core/base/string.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "dear-imgui/api.h"
#include "dear-imgui/inf/widget.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class WidgetGroup;

class ARK_PLUGIN_DEAR_IMGUI_API RendererBuilder {
public:
// [[script::bindings::auto]]
    RendererBuilder();

// [[script::bindings::auto]]
    bool begin(const String& name);
// [[script::bindings::auto]]
    void end();

// [[script::bindings::auto]]
    void bulletText(const String& name);
// [[script::bindings::auto]]
    void button(const String& label);

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
    void showAboutWindow();
// [[script::bindings::auto]]
    void showDemoWindow();

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

        Callback(FuncType func, T arg)
            : _func(std::move(func)), _arg(arg) {
        }

        virtual void render() override {
            _func(_arg());
        }

    private:
        FuncType _func;
        Argument<T> _arg;
    };

private:
    void addBlock(std::function<void(void)> func);

    template<typename T> void addInvocation(std::function<void(typename Argument<T>::ArgType)> func, T arg) {
        current()->addWidget(sp<Invocation<T>>::make(std::move(func), std::move(arg)));
    }

    template<typename T> void addCallback(std::function<bool(typename Argument<T>::ArgType)> func, T arg) {
        current()->addWidget(sp<Callback<T>>::make(std::move(func), std::move(arg)));
    }

    void push(const sp<WidgetGroup>& widget);
    void pop();

    const sp<WidgetGroup>& current() const;

private:
    struct Stub {
        std::stack<sp<WidgetGroup>> _states;
    };

private:
    sp<Stub> _stub;
};


}
}
}

#endif
