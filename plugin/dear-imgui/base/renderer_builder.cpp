#include "dear-imgui/base/renderer_builder.h"

#include <functional>
#include <vector>

#include <imgui.h>

#include "core/base/string.h"

#include "graphics/inf/renderer.h"

#include "dear-imgui/inf/widget.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class WidgetGroup : public Widget {
public:
    virtual ~WidgetGroup() override = default;

    void addWidget(const sp<Widget>& widget) {
        _widgets.push_back(widget);
    }

    virtual void render() override {
        for(const sp<Widget>& i : _widgets)
            i->render();
    }

private:
    std::vector<sp<Widget>> _widgets;
};

namespace {

class Window : public WidgetGroup {
public:
    Window(const String& name, bool open = true)
        : _name(name), _open(open) {
    }

    virtual void render() override {
        if(ImGui::Begin(_name.c_str(), &_open))
            WidgetGroup::render();
        ImGui::End();
    }

private:
    String _name;
    bool _open;
};

class Text : public Widget {
public:
    Text(std::function<void(const char*)> func, const String& content)
        : _func(func), _content(content) {
    }

    virtual void render() override {
        _func(_content.c_str());
    }

private:
    std::function<void(const char*)> _func;
    String _content;
};


class Block : public Widget {
public:
    Block(std::function<void(void)> func)
        : _func(std::move(func)) {
    }

    virtual void render() override {
        _func();
    }

private:
    std::function<void(void)> _func;

};


class DemoWindow : public Widget {
public:
    DemoWindow()
        : _open(true) {
    }

    virtual void render() override {
        ImGui::ShowDemoWindow(&_open);
    }

private:
    bool _open;
};

class RendererWidget : public Renderer {
public:
    RendererWidget(const sp<Widget>& widget)
        : _widget(widget) {
    }

    virtual void render(RenderRequest& /*renderRequest*/, float /*x*/, float /*y*/) override {
        _widget->render();
    }

private:
    sp<Widget> _widget;
};

}

RendererBuilder::RendererBuilder()
    : _stub(sp<Stub>::make())
{
    push(sp<WidgetGroup>::make());
}

bool RendererBuilder::begin(const String& name)
{
    sp<Window> window = sp<Window>::make(name);
    current()->addWidget(window);
    push(window);
    return true;
}

void RendererBuilder::end()
{
    pop();
}

void RendererBuilder::bulletText(const String& content)
{
    current()->addWidget(sp<Text>::make(ImGui::BulletText, content));
}

void RendererBuilder::text(const String& content)
{
    current()->addWidget(sp<Text>::make(ImGui::Text, content));
}

void RendererBuilder::separator()
{
    current()->addWidget(sp<Block>::make(ImGui::Separator));
}

void RendererBuilder::spacing()
{
    current()->addWidget(sp<Block>::make(ImGui::Spacing));
}

void RendererBuilder::showDemoWindow()
{
    current()->addWidget(sp<DemoWindow>::make());
}

sp<Renderer> RendererBuilder::build() const
{
    DCHECK(_stub->_states.size() == 1, "Builder has uncompleted begin/end states");
    return sp<RendererWidget>::make(_stub->_states.top());
}

void RendererBuilder::push(const sp<WidgetGroup>& widget)
{
    _stub->_states.push(widget);
}

void RendererBuilder::pop()
{
    _stub->_states.pop();
}

const sp<WidgetGroup>& RendererBuilder::current() const
{
    return _stub->_states.top();
}

}
}
}
