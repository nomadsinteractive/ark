#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_BUILDER_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_BUILDER_H_

#include <stack>

#include "core/forwarding.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"

#include "dear-imgui/api.h"

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
    void text(const String& name);
// [[script::bindings::auto]]
    void separator();
// [[script::bindings::auto]]
    void spacing();

// [[script::bindings::auto]]
    void showDemoWindow();

// [[script::bindings::auto]]
    sp<Renderer> build() const;

private:
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
