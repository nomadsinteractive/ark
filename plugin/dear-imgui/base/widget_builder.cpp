#include "dear-imgui/base/widget_builder.h"

#include <vector>

#include <imgui.h>

#include "core/impl/variable/variable_wrapper.h"
#include "core/util/string_type.h"

#include "graphics/base/size.h"
#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/impl/vec/vec4_impl.h"

#include "dear-imgui/base/renderer_context.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class WidgetGroup : public Widget {
public:
    virtual ~WidgetGroup() override = default;

    void addWidget(sp<Widget> widget) {
        _widgets.push_back(std::move(widget));
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
    Window(String name, sp<BooleanWrapper> isOpen)
        : _name(std::move(name)), _is_open(std::move(isOpen)) {
    }

    virtual void render() override {
        bool isOpen = _is_open->val();
        if(isOpen) {
            bool isOpenArg = isOpen;
            if(ImGui::Begin(_name.c_str(), &isOpenArg))
                WidgetGroup::render();
            ImGui::End();
            if(isOpen != isOpenArg)
                _is_open->set(isOpenArg);
        }
    }

private:
    String _name;
    sp<BooleanWrapper> _is_open;
};

class WindowNoClose : public WidgetGroup {
public:
    WindowNoClose(String name)
        : _name(std::move(name)) {
    }

    virtual void render() override {
        if(ImGui::Begin(_name.c_str(), nullptr))
            WidgetGroup::render();
        ImGui::End();
    }

private:
    String _name;
};

class WidgetText : public Widget {
public:
    WidgetText(std::function<void(const char*)> func, String content)
        : _func(std::move(func)), _content(std::move(content)) {
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


class FunctionCall : public Widget {
public:
    FunctionCall(std::function<void(void)> func)
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
    Input(std::function<bool(const char*, T*)> func, String label, const sp<Variable<T>>& value)
        : _func(std::move(func)), _label(std::move(label)), _value(value.template as<VariableWrapper<T>>()) {
        DCHECK(_value, "Value should be a Wrapper class");
    }

    virtual void render() override {
        T v = _value->val();
        if(_func(_label.c_str(), &v))
            _value->set(v);
    }

private:
    std::function<bool(const char*, T*)> _func;
    String _label;
    sp<VariableWrapper<T>> _value;
};

template<typename T, typename U> class InputWithType : public Widget {
public:
    InputWithType(std::function<bool(const char*, T*)> func, String label, sp<Variable<T>> value)
        : _label(std::move(label)), _value(std::move(value)), _func(std::move(func)) {
    }

    virtual void render() override {
        T v = _value->val();
        if(_func(_label.c_str(), &v))
        {
            sp<VariableWrapper<T>> wrapper = _value.template as<VariableWrapper<T>>();
            if(wrapper)
                return wrapper->set(v);

            sp<U> impl = _value.template as<U>();
            CHECK(impl, "Mutablable variable required.");
            impl->set(v);
        }
    }

private:
    String _label;
    sp<Variable<T>> _value;
    std::function<bool(const char*, T*)> _func;
};

template<typename T> class WidgetInputText : public Widget {
public:
    WidgetInputText(String label, sp<T> value, size_t maxLength, sp<StringVar> hint, sp<Observer> observer, ImGuiInputTextFlags flags)
        : _label(std::move(label)), _value(std::move(value)), _hint(std::move(hint)), _observer(std::move(observer)), _flags(flags), _text_buf(maxLength) {
        updateInputText();
    }

    virtual void render() override {
        bool renderResult = ImGui::InputTextWithHint(_label.c_str(), _hint ? _hint->val()->c_str() : nullptr, &_text_buf[0], _text_buf.size(), _flags);
        if(renderResult) {
            StringType::set(_value, sp<String>::make(_text_buf.data()));
            if(_observer) {
                _observer->update();
                updateInputText();
            }
        }
        if(_flags & ImGuiInputTextFlags_EnterReturnsTrue) {
            ImGui::SetItemDefaultFocus();
            if (renderResult)
                ImGui::SetKeyboardFocusHere(-1);
        }
    }

private:
    void updateInputText() {
        std::strncpy(&_text_buf[0], _value->val()->c_str(), _text_buf.size());
    }

private:
    std::function<bool(const char*, char*, size_t)> _func;
    String _label;
    sp<T> _value;
    sp<StringVar> _hint;
    sp<Observer> _observer;
    ImGuiInputTextFlags _flags;
    std::vector<char> _text_buf;
};


class DemoWindow : public Widget {
public:
    DemoWindow(std::function<void(bool*)> func, sp<BooleanWrapper> isOpen)
        : _func(std::move(func)), _is_open(std::move(isOpen)) {
    }

    virtual void render() override {
        bool isOpen = _is_open->val();
        if(isOpen) {
            bool isOpenArg = isOpen;
            _func(&isOpenArg);
            if(isOpen != isOpenArg)
                _is_open->set(isOpenArg);
        }
    }

private:
    std::function<void(bool*)> _func;
    sp<BooleanWrapper> _is_open;
};

}


static bool Items_VectorGetter(void* data, int idx, const char** out_text)
{
    const std::vector<String>* items = reinterpret_cast<const std::vector<String>*>(data);
    if (out_text)
        *out_text = items->at(static_cast<size_t>(idx)).c_str();
    return true;
}

WidgetBuilder::WidgetBuilder(const sp<Renderer>& imguiRenderer)
    : _renderer_context(imguiRenderer.as<RendererImgui>()->rendererContext()), _stub(sp<Stub>::make())
{
    push(sp<WidgetGroup>::make());
}

bool WidgetBuilder::begin(String name, sp<Boolean> isOpen)
{
    CHECK(isOpen == nullptr || isOpen.is<BooleanWrapper>(), "isOpen parameter must be either BooleanWrapper instance or nullptr");
    sp<WidgetGroup> window = isOpen ? sp<WidgetGroup>::make<Window>(std::move(name), std::move(isOpen)) : sp<WidgetGroup>::make<WindowNoClose>(std::move(name));
    addWidget(window);
    push(std::move(window));
    return true;
}

void WidgetBuilder::end()
{
    pop();
}

void WidgetBuilder::bulletText(const String& content)
{
    addWidget(sp<WidgetText>::make(ImGui::BulletText, content));
}

sp<Observer> WidgetBuilder::button(const String& label, const V2& size)
{
    sp<Observer> observer = sp<Observer>::make(nullptr, false);
    addCallback<String>([size](const char* s) { return ImGui::Button(s, ImVec2(size.x(), size.y())); }, label, observer);
    return observer;
}

sp<Observer> WidgetBuilder::smallButton(const String& label)
{
    sp<Observer> observer = sp<Observer>::make(nullptr, false);
    addCallback<String>(ImGui::SmallButton, label, observer);
    return observer;
}

void WidgetBuilder::checkbox(const String& label, const sp<Boolean>& option)
{
    addWidget(sp<Input<bool>>::make(ImGui::Checkbox, label, option));
}

void WidgetBuilder::radioButton(const String& label, const sp<Integer>& option, int32_t group)
{
    addWidget(sp<Input<int32_t>>::make([group](const char* l, int32_t* v) { return ImGui::RadioButton(l, v, group); }, label, option));
}

void WidgetBuilder::combo(const String& label, const sp<Integer>& option, const std::vector<String>& items)
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

void WidgetBuilder::listBox(const String& label, const sp<Integer>& option, const std::vector<String>& items)
{
    addWidget(sp<Input<int32_t>>::make([items](const char* l, int32_t* v) { return ImGui::ListBox(l, v, Items_VectorGetter, reinterpret_cast<void*>(const_cast<std::vector<String>*>(&items)), static_cast<int32_t>(items.size())); }, label, option));
}

void WidgetBuilder::indent(float w)
{
    addInvocation<float>(ImGui::Indent, w);
}

void WidgetBuilder::unindent(float w)
{
    addInvocation<float>(ImGui::Unindent, w);
}

void WidgetBuilder::text(const String& content)
{
    addWidget(sp<WidgetText>::make(ImGui::Text, content));
}

sp<Observer> WidgetBuilder::inputText(String label, sp<StringVar::Impl> value, size_t maxLength, int32_t flags)
{
    sp<Observer> observer = sp<Observer>::make(nullptr, false);
    addWidget(sp<WidgetInputText<StringVar::Impl>>::make(std::move(label), std::move(value), maxLength, nullptr, observer, flags));
    return observer;
}

void WidgetBuilder::inputInt(const String& label, const sp<Integer>& value, int32_t step, int32_t step_fast, int32_t flags)
{
    addWidget(sp<Input<int32_t>>::make([step, step_fast, flags](const char* l, int32_t* v) { return ImGui::InputInt(l, v, step, step_fast, flags); }, label, value));
}

void WidgetBuilder::sliderInt(const String& label, const sp<Integer>& value, int32_t vmin, int32_t vmax, const String& format)
{
    addWidget(sp<Input<int32_t>>::make([vmin, vmax, format](const char* l, int32_t* v) { return ImGui::SliderInt(l, v, vmin, vmax, format.c_str()); }, label, value));
}

void WidgetBuilder::inputFloat(const String& label, const sp<Numeric>& value, float step, float step_fast, const String& format)
{
    addWidget(sp<Input<float>>::make([step, step_fast, format](const char* l, float* v) { return ImGui::InputFloat(l, v, step, step_fast, format.c_str()); }, label, value));
}

void WidgetBuilder::inputFloat2(const String& label, const sp<Vec2>& value, const String& format, int32_t flags)
{
    addWidget(sp<InputWithType<V2, Vec2Impl>>::make([format, flags](const char* l, V2* v) { return ImGui::InputFloat2(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void WidgetBuilder::inputFloat3(const String& label, const sp<Size>& size, const String& format, int32_t flags)
{
    inputFloat3(label, size->impl().cast<Vec3>(), format, flags);
}

void WidgetBuilder::inputFloat3(const String& label, const sp<Vec3>& value, const String& format, int32_t flags)
{
    addWidget(sp<InputWithType<V3, Vec3Impl>>::make([format, flags](const char* l, V3* v) { return ImGui::InputFloat3(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void WidgetBuilder::inputFloat4(const String& label, const sp<Vec4>& value, const String& format, int32_t flags)
{
    addWidget(sp<InputWithType<V4, Vec4Impl>>::make([format, flags](const char* l, V4* v) { return ImGui::InputFloat4(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void WidgetBuilder::sliderFloat(const String& label, const sp<Numeric>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<Input<float>>::make([v_min, v_max, format, power](const char* l, float* v) { return ImGui::SliderFloat(l, v, v_min, v_max, format.c_str(), power); }, label, value));
}

void WidgetBuilder::sliderFloat2(const String& label, const sp<Vec2>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<InputWithType<V2, Vec2Impl>>::make([v_min, v_max, format, power](const char* l, V2* v) { return ImGui::SliderFloat2(l, reinterpret_cast<float*>(v), v_min, v_max, format.c_str(), power); }, label, value));
}

void WidgetBuilder::sliderFloat3(const String& label, const sp<Size>& size, float v_min, float v_max, const String& format, float power)
{
    sliderFloat3(label, size->impl().cast<Vec3>(), v_min, v_max, format, power);
}

void WidgetBuilder::sliderFloat3(const String& label, const sp<Vec3>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<InputWithType<V3, Vec3Impl>>::make([v_min, v_max, format, power](const char* l, V3* v) { return ImGui::SliderFloat3(l, reinterpret_cast<float*>(v), v_min, v_max, format.c_str(), power); }, label, value));
}

void WidgetBuilder::sliderFloat4(const String& label, const sp<Vec4>& value, float v_min, float v_max, const String& format, float power)
{
    addWidget(sp<InputWithType<V4, Vec4Impl>>::make([v_min, v_max, format, power](const char* l, V4* v) { return ImGui::SliderFloat4(l, reinterpret_cast<float*>(v), v_min, v_max, format.c_str(), power); }, label, value));
}

void WidgetBuilder::colorEdit3(const String& label, const sp<Vec3>& value)
{
    addWidget(sp<InputWithType<V3, Vec3Impl>>::make([](const char* l, V3* v) { return ImGui::ColorEdit3(l, reinterpret_cast<float*>(v)); }, label, value));
}

void WidgetBuilder::colorEdit4(const String& label, const sp<Vec4>& value)
{
    addWidget(sp<InputWithType<V4, Vec4Impl>>::make([](const char* l, V4* v) { return ImGui::ColorEdit4(l, reinterpret_cast<float*>(v)); }, label, value));
}

void WidgetBuilder::colorPicker3(const String& label, const sp<Vec3>& value)
{
    addWidget(sp<InputWithType<V3, Vec3Impl>>::make([](const char* l, V3* v) { return ImGui::ColorPicker3(l, reinterpret_cast<float*>(v)); }, label, value));
}

void WidgetBuilder::colorPicker4(const String& label, const sp<Vec4>& value)
{
    addWidget(sp<InputWithType<V4, Vec4Impl>>::make([](const char* l, V4* v) { return ImGui::ColorPicker4(l, reinterpret_cast<float*>(v)); }, label, value));
}

void WidgetBuilder::pushID(const String& id)
{
    addFunctionCall([id] { ImGui::PushID(id.c_str()); } );
}

void WidgetBuilder::pushID(int64_t id)
{
    int32_t idHash = static_cast<int32_t>(id) ^ static_cast<int32_t>(id >> 32);
    addFunctionCall([idHash] { ImGui::PushID(idHash); } );
}

void WidgetBuilder::popID()
{
    addFunctionCall(ImGui::PopID);
}

void WidgetBuilder::image(const sp<Texture>& texture, const sp<Vec2>& size, const V2& uv0, const V2& uv1, const sp<Vec4>& color, const sp<Vec4>& borderColor)
{
    addWidget(sp<Image>::make(texture, size, uv0, uv1, color ? color : sp<Vec4>::make<Vec4::Const>(V4(1.0f)), borderColor ? borderColor : sp<Vec4>::make<Vec4::Const>(V4(0)), _renderer_context));
}

void WidgetBuilder::sameLine(float localPosX, float spacingW)
{
    addFunctionCall([localPosX, spacingW] { ImGui::SameLine(localPosX, spacingW); });
}

void WidgetBuilder::separator()
{
    addFunctionCall(ImGui::Separator);
}

void WidgetBuilder::spacing()
{
    addFunctionCall(ImGui::Spacing);
}

sp<Widget> WidgetBuilder::makeAboutWidget(sp<Boolean> isOpen)
{
    return sp<DemoWindow>::make(ImGui::ShowAboutWindow, std::move(isOpen));
}

sp<Widget> WidgetBuilder::makeDemoWidget(sp<Boolean> isOpen)
{
    return sp<DemoWindow>::make(ImGui::ShowDemoWindow, std::move(isOpen));
}

void WidgetBuilder::addFunctionCall(std::function<void()> func)
{
    addWidget(sp<FunctionCall>::make(std::move(func)));
}

void WidgetBuilder::addWidget(sp<Widget> widget)
{
    current()->addWidget(std::move(widget));
}

sp<Widget> WidgetBuilder::makeWidget() const
{
    CHECK(_stub->_states.size() == 1, "Builder has uncompleted begin/end states");
    return _stub->_states.top();
}

void WidgetBuilder::push(sp<WidgetGroup> widgetGroup)
{
    _stub->_states.push(std::move(widgetGroup));
}

void WidgetBuilder::pop()
{
    _stub->_states.pop();
}

const sp<WidgetGroup>& WidgetBuilder::current() const
{
    return _stub->_states.top();
}

}
}
}
