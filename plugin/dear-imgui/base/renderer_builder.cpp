#include "dear-imgui/base/renderer_builder.h"

#include <vector>

#include <imgui.h>

#include "core/impl/variable/variable_wrapper.h"
#include "core/types/global.h"

#include "graphics/base/size.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/util/vec2_type.h"
#include "graphics/util/vec3_type.h"
#include "graphics/util/vec4_type.h"

#include "graphics/inf/renderer.h"

#include "dear-imgui/base/controller.h"
#include "dear-imgui/base/imgui_context.h"
#include "dear-imgui/base/renderer_context.h"
#include "dear-imgui/renderer/renderer_imgui.h"

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
        : _func(std::move(func)), _content(content) {
    }

    virtual void render() override {
        _func(_content.c_str());
    }

private:
    std::function<void(const char*)> _func;
    String _content;
};


class Image : public Widget {
public:
    Image(sp<Texture> texture, sp<Vec2> size, const V2& uv0, const V2& uv1, sp<Vec4> color, sp<Vec4> borderColor, sp<RendererContext> rendererContext)
        : _texture(std::move(texture)), _size(std::move(size)), _uv0(*reinterpret_cast<const ImVec2*>(&uv0)), _uv1(*reinterpret_cast<const ImVec2*>(&uv1)),
          _color(std::move(color)), _border_color(std::move(borderColor)), _renderer_context(std::move(rendererContext)) {
        if(_texture)
            _renderer_context->addTextureRefCount(_texture.get());
    }
    ~Image() override {
        if(_texture)
            _renderer_context->relTextureRefCount(_texture.get());
    }

    virtual void render() override {
        const V2 size = _size->val();
        const V4 color = _color->val();
        const V4 borderColor = _border_color->val();
        ImGui::Image(_texture ? _texture.get() : nullptr, *reinterpret_cast<const ImVec2*>(&size), _uv0, _uv1, *reinterpret_cast<const ImVec4*>(&color), *reinterpret_cast<const ImVec4*>(&borderColor));
    }

private:
    sp<Texture> _texture;
    sp<Vec2> _size;
    ImVec2 _uv0, _uv1;
    sp<Vec4> _color;
    sp<Vec4> _border_color;

    sp<RendererContext> _renderer_context;
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

template<typename T> class Input : public Widget {
public:
    Input(std::function<bool(const char*, T*)> func, const String& label, const sp<Variable<T>>& value)
        : _func(std::move(func)), _label(label), _value(value.template as<VariableWrapper<T>>()) {
        DCHECK(_value, "Value should be a Wrapper class");
    }

    virtual void render() override {
        T v = _value->val();
        if(_func(_label.c_str(), &v))
            _value->set(v);
    }

private:
    std::function<bool(const char*, T*)> _func;
    const String _label;
    sp<VariableWrapper<T>> _value;
};

template<typename T, typename U> class InputVec : public Widget {
public:
    InputVec(std::function<bool(const char*, T*)> func, const String& label, const sp<Variable<T>>& value)
        : _func(std::move(func)), _label(label), _value(value) {
    }

    virtual void render() override {
        T v = _value->val();
        if(_func(_label.c_str(), &v))
            U::set(_value, v);
    }

private:
    std::function<bool(const char*, T*)> _func;
    const String _label;
    sp<Variable<T>> _value;
};

class DemoWindow : public Widget {
public:
    DemoWindow(std::function<void(bool*)> func)
        : _func(std::move(func)), _open(true) {
    }

    virtual void render() override {
        _func(&_open);
    }

private:
    std::function<void(bool*)> _func;

    bool _open;
};

class RendererWidget : public Renderer {
public:
    RendererWidget(const sp<Widget>& widget)
        : _widget(widget) {
    }

    virtual void render(RenderRequest& /*renderRequest*/, const V3& /*position*/) override {
        _widget->render();
    }

private:
    sp<Widget> _widget;
};

}


static bool Items_VectorGetter(void* data, int idx, const char** out_text)
{
    const std::vector<String>* items = reinterpret_cast<const std::vector<String>*>(data);
    if (out_text)
        *out_text = items->at(static_cast<size_t>(idx)).c_str();
    return true;
}

RendererBuilder::RendererBuilder(const sp<Renderer>& imguiRenderer)
    : _renderer_context(imguiRenderer.as<RendererImgui>()->rendererContext()), _stub(sp<Stub>::make())
{
    push(sp<WidgetGroup>::make());
}

bool RendererBuilder::begin(const String& name)
{
    sp<Window> window = sp<Window>::make(name);
    addWidget(window);
    push(window);
    return true;
}

void RendererBuilder::end()
{
    pop();
}

void RendererBuilder::bulletText(const String& content)
{
    addWidget(sp<Text>::make(ImGui::BulletText, content));
}

sp<Observer> RendererBuilder::button(const String& label, const V2& size)
{
    sp<Observer> observer = sp<Observer>::make(nullptr, false);
    addCallback<String>([size](const char* s) { return ImGui::Button(s, ImVec2(size.x(), size.y())); }, label, observer);
    return observer;
}

sp<Observer> RendererBuilder::smallButton(const String& label)
{
    sp<Observer> observer = sp<Observer>::make(nullptr, false);
    addCallback<String>(ImGui::SmallButton, label, observer);
    return observer;
}

void RendererBuilder::checkbox(const String& label, const sp<Boolean>& option)
{
    addWidget(sp<Input<bool>>::make(ImGui::Checkbox, label, option));
}

void RendererBuilder::radioButton(const String& label, const sp<Integer>& option, int32_t group)
{
    addWidget(sp<Input<int32_t>>::make([group](const char* l, int32_t* v) { return ImGui::RadioButton(l, v, group); }, label, option));
}

void RendererBuilder::combo(const String& label, const sp<Integer>& option, const std::vector<String>& items)
{
    StringBuffer sb;
    for(const String& i : items)
    {
        sb << i;
        sb << '\0';
    }
    sb << '\0';

    const String s = sb.str();
    addWidget(sp<Input<int32_t>>::make([s](const char* l, int32_t* v) { return ImGui::Combo(l, v, s.c_str()); }, label, option));
}

void RendererBuilder::listBox(const String& label, const sp<Integer>& option, const std::vector<String>& items)
{
    addWidget(sp<Input<int32_t>>::make([items](const char* l, int32_t* v) { return ImGui::ListBox(l, v, Items_VectorGetter, (void*) &items, items.size()); }, label, option));
}

void RendererBuilder::indent(float w)
{
    addInvocation<float>(ImGui::Indent, w);
}

void RendererBuilder::unindent(float w)
{
    addInvocation<float>(ImGui::Unindent, w);
}

void RendererBuilder::text(const String& content)
{
    addWidget(sp<Text>::make(ImGui::Text, content));
}

void RendererBuilder::inputInt(const String& label, const sp<Integer>& value, int32_t step, int32_t step_fast, int32_t flags)
{
    addWidget(sp<Input<int32_t>>::make([step, step_fast, flags](const char* l, int32_t* v) { return ImGui::InputInt(l, v, step, step_fast, flags); }, label, value));
}

void RendererBuilder::inputFloat(const String& label, const sp<Numeric>& value, float step, float step_fast, const String& format)
{
    addWidget(sp<Input<float>>::make([step, step_fast, format](const char* l, float* v) { return ImGui::InputFloat(l, v, step, step_fast, format.c_str()); }, label, value));
}

void RendererBuilder::inputFloat2(const String& label, const sp<Vec2>& value, const String& format, int32_t flags)
{
    addWidget(sp<InputVec<V2, Vec2Type>>::make([format, flags](const char* l, V2* v) { return ImGui::InputFloat2(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void RendererBuilder::inputFloat3(const String& label, const sp<Size>& size, const String& format, int32_t flags)
{
    inputFloat3(label, size->impl().cast<Vec3>(), format, flags);
}

void RendererBuilder::inputFloat3(const String& label, const sp<Vec3>& value, const String& format, int32_t flags)
{
    addWidget(sp<InputVec<V3, Vec3Type>>::make([format, flags](const char* l, V3* v) { return ImGui::InputFloat3(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void RendererBuilder::inputFloat4(const String& label, const sp<Vec4>& value, const String& format, int32_t flags)
{
    addWidget(sp<InputVec<V4, Vec4Type>>::make([format, flags](const char* l, V4* v) { return ImGui::InputFloat4(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void RendererBuilder::sliderFloat(const String& label, const sp<Numeric>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<Input<float>>::make([v_min, v_max, format, power](const char* l, float* v) { return ImGui::SliderFloat(l, v, v_min, v_max, format.c_str(), power); }, label, value));
}

void RendererBuilder::sliderFloat2(const String& label, const sp<Vec2>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<InputVec<V2, Vec2Type>>::make([v_min, v_max, format, power](const char* l, V2* v) { return ImGui::SliderFloat2(l, reinterpret_cast<float*>(v), v_min, v_max, format.c_str(), power); }, label, value));
}

void RendererBuilder::sliderFloat3(const String& label, const sp<Size>& size, float v_min, float v_max, const String& format, float power)
{
    sliderFloat3(label, size->impl().cast<Vec3>(), v_min, v_max, format, power);
}

void RendererBuilder::sliderFloat3(const String& label, const sp<Vec3>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<InputVec<V3, Vec3Type>>::make([v_min, v_max, format, power](const char* l, V3* v) { return ImGui::SliderFloat3(l, reinterpret_cast<float*>(v), v_min, v_max, format.c_str(), power); }, label, value));
}

void RendererBuilder::sliderFloat4(const String& label, const sp<Vec4>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<InputVec<V4, Vec4Type>>::make([v_min, v_max, format, power](const char* l, V4* v) { return ImGui::SliderFloat4(l, reinterpret_cast<float*>(v), v_min, v_max, format.c_str(), power); }, label, value));
}

void RendererBuilder::colorEdit3(const String& label, const sp<Vec3>& value)
{
    addWidget(sp<InputVec<V3, Vec3Type>>::make([](const char* l, V3* v) { return ImGui::ColorEdit3(l, reinterpret_cast<float*>(v)); }, label, value));
}

void RendererBuilder::colorEdit4(const String& label, const sp<Vec4>& value)
{
    addWidget(sp<InputVec<V4, Vec4Type>>::make([](const char* l, V4* v) { return ImGui::ColorEdit4(l, reinterpret_cast<float*>(v)); }, label, value));
}

void RendererBuilder::colorPicker3(const String& label, const sp<Vec3>& value)
{
    addWidget(sp<InputVec<V3, Vec3Type>>::make([](const char* l, V3* v) { return ImGui::ColorPicker3(l, reinterpret_cast<float*>(v)); }, label, value));
}

void RendererBuilder::colorPicker4(const String& label, const sp<Vec4>& value)
{
    addWidget(sp<InputVec<V4, Vec4Type>>::make([](const char* l, V4* v) { return ImGui::ColorPicker4(l, reinterpret_cast<float*>(v)); }, label, value));
}

void RendererBuilder::image(const sp<Texture>& texture, const sp<Vec2>& size, const V2& uv0, const V2& uv1, const sp<Vec4>& color, const sp<Vec4>& borderColor)
{
    addWidget(sp<Image>::make(texture, size, uv0, uv1, color ? color : sp<Vec4>::make<Vec4::Const>(V4(1.0f)), borderColor ? borderColor : sp<Vec4>::make<Vec4::Const>(V4(0)), _renderer_context));
}

sp<Controller> RendererBuilder::showAboutWindow()
{
    const sp<Controller> controller = sp<Controller>::make(sp<DemoWindow>::make(ImGui::ShowAboutWindow));
    addWidget(controller);
    return controller;
}

void RendererBuilder::sameLine(float localPosX, float spacingW)
{
    addBlock([localPosX, spacingW] { ImGui::SameLine(localPosX, spacingW); });
}

void RendererBuilder::separator()
{
    addBlock(ImGui::Separator);
}

void RendererBuilder::spacing()
{
    addBlock(ImGui::Spacing);
}

sp<Controller> RendererBuilder::showDemoWindow()
{
    const sp<Controller> controller = sp<Controller>::make(sp<DemoWindow>::make(ImGui::ShowDemoWindow));
    addWidget(controller);
    return controller;
}

sp<Renderer> RendererBuilder::build() const
{
    DCHECK(_stub->_states.size() == 1, "Builder has uncompleted begin/end states");
    return sp<RendererWidget>::make(_stub->_states.top());
}

void RendererBuilder::addBlock(std::function<void()> func)
{
    addWidget(sp<Block>::make(std::move(func)));
}

void RendererBuilder::addWidget(const sp<Widget>& widget)
{
    current()->addWidget(widget);
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
