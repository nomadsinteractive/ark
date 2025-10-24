#include "dear-imgui/base/widget_builder.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "core/base/string_buffer.h"
#include "core/impl/variable/variable_wrapper.h"
#include "core/util/string_type.h"

#include "graphics/components/size.h"
#include "graphics/impl/vec/vec2_impl.h"
#include "graphics/impl/vec/vec3_impl.h"
#include "graphics/impl/vec/vec4_impl.h"
#include "graphics/util/vec3_type.h"

#include "dear-imgui/base/renderer_context.h"
#include "dear-imgui/base/widget_type.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark::plugin::dear_imgui {

const ImGuiDataTypeInfo GDataTypeInfo[] =
{
    { sizeof(char),             "S8",   "%d",   "%d"    },  // ImGuiDataType_S8
    { sizeof(unsigned char),    "U8",   "%u",   "%u"    },
    { sizeof(short),            "S16",  "%d",   "%d"    },  // ImGuiDataType_S16
    { sizeof(unsigned short),   "U16",  "%u",   "%u"    },
    { sizeof(int),              "S32",  "%d",   "%d"    },  // ImGuiDataType_S32
    { sizeof(unsigned int),     "U32",  "%u",   "%u"    },
#ifdef _MSC_VER
    { sizeof(ImS64),            "S64",  "%I64d","%I64d" },  // ImGuiDataType_S64
    { sizeof(ImU64),            "U64",  "%I64u","%I64u" },
#else
    { sizeof(ImS64),            "S64",  "%lld", "%lld"  },  // ImGuiDataType_S64
    { sizeof(ImU64),            "U64",  "%llu", "%llu"  },
#endif
    { sizeof(float),            "float", "%.3f","%f"    },  // ImGuiDataType_Float (float are promoted to double in va_arg)
    { sizeof(double),           "double","%f",  "%lf"   },  // ImGuiDataType_Double
    { sizeof(bool),             "bool", "%d",   "%d"    },  // ImGuiDataType_Bool
    { 0,                        "char*","%s",   "%s"    },  // ImGuiDataType_String
};

bool SliderScalarN(const char* label, ImGuiDataType data_type, void* v, int components, const void* v_min, const void* v_max, const char* format, ImGuiSliderFlags flags = 0)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    bool value_changed = false;
    ImGui::BeginGroup();
    ImGui::PushID(label);
    ImGui::PushMultiItemsWidths(components, ImGui::CalcItemWidth());
    size_t type_size = GDataTypeInfo[data_type].Size;
    for (int i = 0; i < components; i++)
    {
        ImGui::PushID(i);
        if (i > 0)
            ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
        value_changed |= ImGui::SliderScalar("", data_type, v, v_min, v_max, format, flags);
        ImGui::PopID();
        ImGui::PopItemWidth();
        v = (void*)((char*)v + type_size);
        v_min = (const void*)((const char*)v_min + type_size);
        v_max = (const void*)((const char*)v_max + type_size);
    }
    ImGui::PopID();

    const char* label_end = ImGui::FindRenderedTextEnd(label);
    if (label != label_end)
    {
        ImGui::SameLine(0, g.Style.ItemInnerSpacing.x);
        ImGui::TextEx(label, label_end);
    }

    ImGui::EndGroup();
    return value_changed;
}

class WidgetGroup : public Widget {
public:
    ~WidgetGroup() override = default;

    void addWidget(sp<Widget> widget) {
        _widgets.push_back(std::move(widget));
    }

    void render() override {
        for(const sp<Widget>& i : _widgets)
            i->render();
    }

private:
    Vector<sp<Widget>> _widgets;
};

namespace {

class Window final : public WidgetGroup {
public:
    Window(String name, sp<BooleanWrapper> isOpen)
        : _name(std::move(name)), _is_open(std::move(isOpen)) {
    }

    void render() override {
        if(const bool isOpen = _is_open->val()) {
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

class MainMenuBar final : public WidgetGroup {
public:
    MainMenuBar(const sp<Boolean::Impl>& returnValue)
        : _return_value(returnValue) {
    }

    void render() override
    {
        const bool r = ImGui::BeginMainMenuBar();
        if(r)
        {
            WidgetGroup::render();
            ImGui::EndMainMenuBar();
        }
        _return_value->set(r);
    }

private:
    sp<Boolean::Impl> _return_value;
};

class Menu final : public WidgetGroup {
public:
    Menu(String label, sp<Boolean> enabled, const sp<Boolean::Impl>& returnValue)
        : _label(std::move(label)), _enabled(std::move(enabled), true), _return_value(returnValue) {
    }

    void render() override
    {
        const bool r = ImGui::BeginMenu(_label.c_str(), _enabled.val());
        if(r)
        {
            WidgetGroup::render();
            ImGui::EndMenu();
        }
        _return_value->set(r);
    }

private:
    String _label;
    OptionalVar<Boolean> _enabled;
    sp<Boolean::Impl> _return_value;
};

class MenuItem final : public Widget {
public:
    MenuItem(String label, String shortcut, sp<Boolean> selected, sp<Boolean> enabled, sp<Runnable> observer, const bool toggleable)
        : _label(std::move(label)), _shortcut(std::move(shortcut)), _selected(std::move(selected)), _enabled(std::move(enabled), true), _observer(std::move(observer)), _toggleable(toggleable) {
        ASSERT(_selected);
    }

    void render() override
    {
        bool r;
        if(_toggleable)
        {
            bool selected = _selected->val();
            r = ImGui::MenuItem(_label.c_str(), _shortcut.c_str(), &selected, _enabled.val());
            BooleanType::set(_selected, selected);
        }
        else
            r = ImGui::MenuItem(_label.c_str(), _shortcut.c_str(), _selected->val(), _enabled.val());

        if(r)
            _observer->run();
    }

private:
    String _label;
    String _shortcut;
    sp<Boolean> _selected;
    OptionalVar<Boolean> _enabled;
    sp<Runnable> _observer;
    bool _toggleable;
};

class TreeNode final : public WidgetGroup {
public:
    TreeNode(const sp<Boolean::Impl>& returnValue, String label)
        : _return_value(returnValue), _label(std::move(label)) {
    }

    void render() override
    {
        const bool r = ImGui::TreeNode(_label.c_str());
        if(r)
        {
            WidgetGroup::render();
            ImGui::TreePop();
        }
        _return_value->set(r);
    }

private:
    sp<Boolean::Impl> _return_value;
    String _label;
};

class Table final : public WidgetGroup {
public:
    Table(const sp<Boolean::Impl>& returnValue, String strId, const int32_t columns, const int32_t flags)
        : _return_value(returnValue), _str_id(std::move(strId)), _columns(columns), _flags(flags) {
    }

    void render() override
    {
        const bool r = ImGui::BeginTable(_str_id.c_str(), _columns, _flags);
        if(r)
        {
            WidgetGroup::render();
            ImGui::EndTable();
        }
        _return_value->set(r);
    }

private:
    sp<Boolean::Impl> _return_value;
    String _str_id;
    int32_t _columns;
    int32_t _flags;
};

class TabBar final : public WidgetGroup {
public:
    TabBar(String name, ImGuiTabBarFlags flags, sp<BooleanWrapper> isOpen)
        : _str_id(std::move(name)), _flags(flags), _is_open(std::move(isOpen)) {
    }

    void render() override {
        const bool isOpen = ImGui::BeginTabBar(_str_id.c_str(), _flags);
        if(isOpen)
        {
            WidgetGroup::render();
            ImGui::EndTabBar();
        }
        if(isOpen != _is_open->val())
            _is_open->set(isOpen);
    }

private:
    String _str_id;
    ImGuiTabBarFlags _flags;
    sp<BooleanWrapper> _is_open;
};

class TabItem final : public WidgetGroup {
public:
    TabItem(String title, ImGuiTabItemFlags flags, sp<BooleanWrapper> activated, sp<BooleanWrapper> opened)
        : _title(std::move(title)), _flags(flags), _activated(std::move(activated)), _opened(std::move(opened)) {
    }

    void render() override {
        if(bool isOpen = _opened ? _opened->val() : true) {
            const bool activated = ImGui::BeginTabItem(_title.c_str(), _opened ? &isOpen : nullptr, _flags);
            if(activated)
            {
                WidgetGroup::render();
                ImGui::EndTabItem();
            }
            _activated->set(activated);
            if(_opened)
                _opened->set(isOpen);
        }
    }

private:
    String _title;
    ImGuiTabItemFlags _flags;
    sp<BooleanWrapper> _activated;
    sp<BooleanWrapper> _opened;
};

class WindowNoClose final : public WidgetGroup {
public:
    WindowNoClose(String name)
        : _name(std::move(name)) {
    }

    void render() override {
        if(ImGui::Begin(_name.c_str(), nullptr))
            WidgetGroup::render();
        ImGui::End();
    }

private:
    String _name;
};

class WidgetText final : public Widget {
public:
    WidgetText(std::function<void(const char*)> func, String content)
        : _func(std::move(func)), _content(std::move(content)) {
    }

    void render() override {
        _func(_content.c_str());
    }

private:
    std::function<void(const char*)> _func;
    String _content;
};

class Image final : public Widget {
public:
    Image(sp<Texture> texture, sp<Vec2> size, const V2& uv0, const V2& uv1, sp<Vec4> color, sp<Vec4> borderColor, sp<RendererContext> rendererContext)
        : _texture(std::move(texture)), _size(!size && _texture ? Vec3Type::xy(_texture->size()) : std::move(size)), _uv0(*reinterpret_cast<const ImVec2*>(&uv0)), _uv1(*reinterpret_cast<const ImVec2*>(&uv1)),
          _color(std::move(color), {1.0f, 1.0f, 1.0f, 1.0f}), _border_color(std::move(borderColor)), _renderer_context(std::move(rendererContext)) {
        if(_texture)
            _renderer_context->addTextureRef(_texture);
        ASSERT(_size);
    }
    ~Image() override {
        if(_texture)
            _renderer_context->releaseTextureRef(_texture);
    }

    void render() override {
        const V2 size = _size->val();
        const V4 color = _color.val();
        const V4 borderColor = _border_color.val();
        ImGui::Image(reinterpret_cast<ImTextureID>(_texture ? _texture.get() : nullptr), *reinterpret_cast<const ImVec2*>(&size), _uv0, _uv1, *reinterpret_cast<const ImVec4*>(&color), *reinterpret_cast<const ImVec4*>(&borderColor));
    }

private:
    sp<Texture> _texture;
    sp<Vec2> _size;
    ImVec2 _uv0, _uv1;
    OptionalVar<Vec4> _color;
    OptionalVar<Vec4> _border_color;

    sp<RendererContext> _renderer_context;
};

class GetImVec2 final : public Widget, public Vec2 {
public:
    GetImVec2(std::function<ImVec2()> func)
        : _func(std::move(func)) {
    }

    void render() override
    {
        const ImVec2 vec2 = _func();
        _value = {vec2.x, vec2.y};
    }

    bool update(uint64_t timestamp) override
    {
        return true;
    }

    V2 val() override
    {
        return _value;
    }

private:
    std::function<ImVec2()> _func;
    V2 _value;
};

class FunctionCall final : public Widget {
public:
    FunctionCall(std::function<void()> func)
        : _func(std::move(func)) {
    }

    void render() override {
        _func();
    }

private:
    std::function<void()> _func;
};

template<typename T> class Input final : public Widget {
public:
    Input(std::function<bool(const char*, T*)> func, String label, const sp<Variable<T>>& value)
        : _func(std::move(func)), _label(std::move(label)), _value(value.template ensureInstance<VariableWrapper<T>>("Value should be a Wrapper class")) {
    }

    void render() override {
        T v = _value->val();
        if(_func(_label.c_str(), &v))
            _value->set(v);
    }

private:
    std::function<bool(const char*, T*)> _func;
    String _label;
    sp<VariableWrapper<T>> _value;
};

template<typename T, typename U> class InputWithType final : public Widget {
public:
    InputWithType(std::function<bool(const char*, T*)> func, String label, sp<Variable<T>> value)
        : _label(std::move(label)), _value(std::move(value)), _func(std::move(func)) {
    }

    void render() override {
        T v = _value->val();
        if(_func(_label.c_str(), &v))
        {
            if(const sp<VariableWrapper<T>> wrapper = _value.template asInstance<VariableWrapper<T>>())
                return wrapper->set(v);

            const sp<U> impl = _value.template ensureInstance<U>("Mutablable variable required.");
            impl->set(v);
        }
    }

private:
    String _label;
    sp<Variable<T>> _value;
    std::function<bool(const char*, T*)> _func;
};

template<typename T, typename IMPL> class InputWithScalarN final : public Widget {
public:
    typedef std::function<bool(const char* label, void* p_data, const void* p_min, const void* p_max, const char* format)> FuncType;

    InputWithScalarN(FuncType func, String label, sp<Variable<T>> value, sp<Variable<T>> vmin, sp<Variable<T>> vmax, Optional<String> format)
        :  _func(std::move(func)), _label(std::move(label)), _value(std::move(value)), _vmin(std::move(vmin)), _vmax(std::move(vmax)), _format(std::move(format)) {
    }

    void render() override {
        T v = _value->val();
        const T vmin = _vmin->val();
        const T vmax = _vmax->val();
        if(_func(_label.c_str(), &v, &vmin, &vmax, _format ? _format->c_str() : nullptr))
        {
            if(const sp<VariableWrapper<T>> wrapper = _value.template asInstance<VariableWrapper<T>>())
                return wrapper->set(v);

            const sp<IMPL> impl = _value.template ensureInstance<IMPL>("Mutablable variable required.");
            impl->set(v);
        }
    }

private:
    FuncType _func;
    String _label;
    sp<Variable<T>> _value;
    sp<Variable<T>> _vmin;
    sp<Variable<T>> _vmax;
    Optional<String> _format;
};

template<typename T> class WidgetInputText final : public Widget {
public:
    WidgetInputText(String label, sp<T> value, const size_t maxLength, sp<StringVar> hint, sp<Observer> observer, const ImGuiInputTextFlags flags)
        : _label(std::move(label)), _value(std::move(value)), _hint(std::move(hint)), _observer(std::move(observer)), _flags(flags), _text_buf(maxLength) {
        updateInputText();
    }

    void render() override {
        const bool renderResult = ImGui::InputTextWithHint(_label.c_str(), _hint ? _hint->val().data() : nullptr, &_text_buf[0], _text_buf.size(), _flags);
        if(renderResult) {
            StringType::set(_value, _text_buf.data());
            if(_observer) {
                _observer->notify();
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
        std::strncpy(&_text_buf[0], _value->val().data(), _text_buf.size());
    }

private:
    std::function<bool(const char*, char*, size_t)> _func;
    String _label;
    sp<T> _value;
    sp<StringVar> _hint;
    sp<Observer> _observer;
    ImGuiInputTextFlags _flags;
    Vector<char> _text_buf;
};


class DemoWindow final : public Widget {
public:
    DemoWindow(std::function<void(bool*)> func, sp<BooleanWrapper> isOpen)
        : _func(std::move(func)), _is_open(std::move(isOpen)) {
    }

    void render() override {
        if(const bool isOpen = _is_open->val())
        {
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

bool Items_VectorGetter(void* data, int idx, const char** out_text)
{
    const auto items = static_cast<const Vector<String>*>(data);
    if (out_text)
        *out_text = items->at(static_cast<size_t>(idx)).c_str();
    return true;
}

}

WidgetBuilder::WidgetBuilder(const sp<Renderer>& imguiRenderer)
    : _renderer_context(imguiRenderer.ensureInstance<RendererImgui>()->rendererContext()), _stub(sp<Stub>::make())
{
    push(sp<WidgetGroup>::make());
}

bool WidgetBuilder::begin(String name, sp<Boolean> isOpen)
{
    CHECK(isOpen == nullptr || isOpen.isInstance<BooleanWrapper>(), "isOpen parameter must be either BooleanWrapper instance or nullptr");
    sp<WidgetGroup> window = isOpen ? sp<WidgetGroup>::make<Window>(std::move(name), std::move(isOpen)) : sp<WidgetGroup>::make<WindowNoClose>(std::move(name));
    addWidgetGroupAndPush(std::move(window));
    return true;
}

void WidgetBuilder::end()
{
    pop();
}

sp<Vec2> WidgetBuilder::getCursorScreenPos()
{
    const sp<GetImVec2> widget = sp<GetImVec2>::make(ImGui::GetCursorScreenPos);
    addWidget(widget);
    return widget;
}

sp<Vec2> WidgetBuilder::getContentRegionAvail()
{
    const sp<GetImVec2> widget = sp<GetImVec2>::make(ImGui::GetContentRegionAvail);
    addWidget(widget);
    return widget;
}

sp<Vec2> WidgetBuilder::getItemRectMin()
{
    const sp<GetImVec2> widget = sp<GetImVec2>::make(ImGui::GetItemRectMin);
    addWidget(widget);
    return widget;
}

sp<Vec2> WidgetBuilder::getItemRectMax()
{
    const sp<GetImVec2> widget = sp<GetImVec2>::make(ImGui::GetItemRectMax);
    addWidget(widget);
    return widget;
}

void WidgetBuilder::bulletText(String content)
{
    addWidget(sp<Widget>::make<WidgetText>(ImGui::BulletText, std::move(content)));
}

sp<Observer> WidgetBuilder::button(const String& label, const V2 size)
{
    sp<Observer> observer = sp<Observer>::make();
    addCallback<String>([size](const char* s) { return ImGui::Button(s, ImVec2(size.x(), size.y())); }, label, observer);
    return observer;
}

sp<Observer> WidgetBuilder::smallButton(const String& label)
{
    sp<Observer> observer = sp<Observer>::make();
    addCallback<String>(ImGui::SmallButton, label, observer);
    return observer;
}

sp<Boolean> WidgetBuilder::treeNode(String label)
{
    sp<Boolean::Impl> returnValue = sp<Boolean::Impl>::make(true);
    addWidgetGroupAndPush(sp<WidgetGroup>::make<TreeNode>(returnValue, std::move(label)));
    return returnValue;
}

void WidgetBuilder::treePop()
{
    pop();
}

void WidgetBuilder::checkbox(const String& label, const sp<Boolean>& option)
{
    addWidget(sp<Widget>::make<Input<bool>>(ImGui::Checkbox, label, option));
}

void WidgetBuilder::radioButton(const String& label, const sp<Integer>& option, int32_t group)
{
    addWidget(sp<Widget>::make<Input<int32_t>>([group](const char* l, int32_t* v) { return ImGui::RadioButton(l, v, group); }, label, option));
}

void WidgetBuilder::combo(const String& label, const sp<Integer>& option, const Vector<String>& items)
{
    StringBuffer sb;
    for(const String& i : items)
    {
        sb << i;
        sb << '\0';
    }
    sb << '\0';

    const String s = sb.str();
    addWidget(sp<Widget>::make<Input<int32_t>>([s](const char* l, int32_t* v) { return ImGui::Combo(l, v, s.c_str()); }, label, option));
}

void WidgetBuilder::listBox(const String& label, const sp<Integer>& option, const Vector<String>& items)
{
    addWidget(sp<Widget>::make<Input<int32_t>>([items](const char* l, int32_t* v) { return ImGui::ListBox(l, v, Items_VectorGetter, reinterpret_cast<void*>(const_cast<Vector<String>*>(&items)), static_cast<int32_t>(items.size())); }, label, option));
}

void WidgetBuilder::indent(const float w)
{
    addInvocation<float>(ImGui::Indent, w);
}

void WidgetBuilder::unindent(const float w)
{
    addInvocation<float>(ImGui::Unindent, w);
}

void WidgetBuilder::text(String text)
{
    addWidget(sp<Widget>::make<WidgetText>(ImGui::Text, std::move(text)));
}

void WidgetBuilder::textWrapped(String text)
{
    addWidget(sp<Widget>::make<WidgetText>(ImGui::TextWrapped, std::move(text)));
}

sp<Observer> WidgetBuilder::inputText(String label, sp<StringVar> value, size_t maxLength, int32_t flags)
{
    sp<Observer> observer = sp<Observer>::make();
    addWidget(sp<Widget>::make<WidgetInputText<StringVar>>(std::move(label), std::move(value), maxLength, nullptr, observer, flags));
    return observer;
}

void WidgetBuilder::inputInt(const String& label, const sp<Integer>& value, int32_t step, int32_t stepFast, int32_t flags)
{
    addWidget(sp<Widget>::make<Input<int32_t>>([step, stepFast, flags](const char* l, int32_t* v) { return ImGui::InputInt(l, v, step, stepFast, flags); }, label, value));
}

void WidgetBuilder::sliderInt(const String& label, const sp<Integer>& value, const int32_t vMin, const int32_t vMax, const String& format)
{
    addWidget(sp<Widget>::make<Input<int32_t>>([vMin, vMax, format](const char* l, int32_t* v) { return ImGui::SliderInt(l, v, vMin, vMax, format.c_str()); }, label, value));
}

void WidgetBuilder::inputFloat(const String& label, const sp<Numeric>& value, float step, float step_fast, const String& format, const Imgui::ImGuiInputTextFlags flags)
{
    addWidget(sp<Widget>::make<Input<float>>([step, step_fast, format, flags](const char* l, float* v) { return ImGui::InputFloat(l, v, step, step_fast, format.c_str(), flags); }, label, value));
}

void WidgetBuilder::inputFloat2(const String& label, const sp<Vec2>& value, const String& format, const Imgui::ImGuiInputTextFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V2, Vec2Impl>>([format, flags](const char* l, V2* v) { return ImGui::InputFloat2(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void WidgetBuilder::inputFloat3(const String& label, const sp<Size>& value, const String& format, const Imgui::ImGuiInputTextFlags flags)
{
    inputFloat3(label, value->impl().cast<Vec3>(), format, flags);
}

void WidgetBuilder::inputFloat3(const String& label, const sp<Vec3>& value, const String& format, const Imgui::ImGuiInputTextFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V3, Vec3Impl>>([format, flags](const char* l, V3* v) { return ImGui::InputFloat3(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void WidgetBuilder::inputFloat4(const String& label, const sp<Vec4>& value, const String& format, const Imgui::ImGuiInputTextFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V4, Vec4Impl>>([format, flags](const char* l, V4* v) { return ImGui::InputFloat4(l, reinterpret_cast<float*>(v), format.c_str(), flags); }, label, value));
}

void WidgetBuilder::sliderFloat(String label, const sp<Numeric>& value, float vMin, float vMax, const String& format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<Input<float>>([vMin, vMax, format, flags](const char* l, float* v) { return ImGui::SliderFloat(l, v, vMin, vMax, format.c_str(), flags); }, std::move(label), value));
}

void WidgetBuilder::sliderFloat2(String label, const sp<Vec2>& value, float vMin, float vMax, const String& format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V2, Vec2Impl>>([vMin, vMax, format, flags](const char* l, V2* v) { return ImGui::SliderFloat2(l, reinterpret_cast<float*>(v), vMin, vMax, format.c_str(), flags); }, std::move(label), value));
}

void WidgetBuilder::sliderFloat3(String label, const sp<Size>& value, const float vMin, const float vMax, const String& format, const Imgui::ImGuiSliderFlags flags)
{
    sliderFloat3(std::move(label), value->impl().cast<Vec3>(), vMin, vMax, format, flags);
}

void WidgetBuilder::sliderFloat3(String label, const sp<Vec3>& value, float vMin, float vMax, const String& format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V3, Vec3Impl>>([vMin, vMax, format, flags](const char* l, V3* v) { return ImGui::SliderFloat3(l, reinterpret_cast<float*>(v), vMin, vMax, format.c_str(), flags); }, std::move(label), value));
}

void WidgetBuilder::sliderFloat4(String label, const sp<Vec4>& value, float vMin, float vMax, const String& format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V4, Vec4Impl>>([vMin, vMax, format, flags](const char* l, V4* v) { return ImGui::SliderFloat4(l, reinterpret_cast<float*>(v), vMin, vMax, format.c_str(), flags); }, std::move(label), value));
}

void WidgetBuilder::sliderScalar(String label, sp<Vec2> value, sp<Vec2> vMin, sp<Vec2> vMax, Optional<String> format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<InputWithScalarN<V2, Vec2Impl>>([flags](const char* label, void* p_data, const void* p_min, const void* p_max, const char* format) {
        return SliderScalarN(label, ImGuiDataType_Float, p_data, 2, p_min, p_max, format, flags);
    }, std::move(label), std::move(value), std::move(vMin), std::move(vMax), std::move(format)));
}

void WidgetBuilder::sliderScalar(String label, sp<Vec3> value, sp<Vec3> vMin, sp<Vec3> vMax, Optional<String> format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<InputWithScalarN<V3, Vec3Impl>>([flags](const char* label, void* p_data, const void* p_min, const void* p_max, const char* format) {
        return SliderScalarN(label, ImGuiDataType_Float, p_data, 3, p_min, p_max, format, flags);
    }, std::move(label), std::move(value), std::move(vMin), std::move(vMax), std::move(format)));
}

void WidgetBuilder::sliderScalar(String label, sp<Vec4> value, sp<Vec4> vMin, sp<Vec4> vMax, Optional<String> format, const Imgui::ImGuiSliderFlags flags)
{
    addWidget(sp<Widget>::make<InputWithScalarN<V4, Vec4Impl>>([flags](const char* label, void* p_data, const void* p_min, const void* p_max, const char* format) {
        return SliderScalarN(label, ImGuiDataType_Float, p_data, 4, p_min, p_max, format, flags);
    }, std::move(label), std::move(value), std::move(vMin), std::move(vMax), std::move(format)));
}

void WidgetBuilder::colorEdit3(const String& label, const sp<Vec3>& value, const Imgui::ImGuiColorEditFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V3, Vec3Impl>>([flags](const char* l, V3* v) { return ImGui::ColorEdit3(l, reinterpret_cast<float*>(v), flags); }, label, value));
}

void WidgetBuilder::colorEdit4(const String& label, const sp<Vec4>& value, const Imgui::ImGuiColorEditFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V4, Vec4Impl>>([flags](const char* l, V4* v) { return ImGui::ColorEdit4(l, reinterpret_cast<float*>(v), flags); }, label, value));
}

void WidgetBuilder::colorPicker3(const String& label, const sp<Vec3>& value, const Imgui::ImGuiColorEditFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V3, Vec3Impl>>([flags](const char* l, V3* v) { return ImGui::ColorPicker3(l, reinterpret_cast<float*>(v), flags); }, label, value));
}

void WidgetBuilder::colorPicker4(const String& label, const sp<Vec4>& value, const Imgui::ImGuiColorEditFlags flags)
{
    addWidget(sp<Widget>::make<InputWithType<V4, Vec4Impl>>([flags](const char* l, V4* v) { return ImGui::ColorPicker4(l, reinterpret_cast<float*>(v), flags); }, label, value));
}

sp<Boolean> WidgetBuilder::beginTable(String strId, int32_t columns, const Imgui::ImGuiTableFlags flags)
{
    sp<Boolean::Impl> returnValue = sp<Boolean::Impl>::make(true);
    addWidgetGroupAndPush(sp<WidgetGroup>::make<Table>(returnValue, std::move(strId), columns, flags));
    return returnValue;
}

void WidgetBuilder::endTable()
{
    pop();
}

void WidgetBuilder::tableSetupColumn(String label)
{
    addFunctionCall([label = std::move(label)] { ImGui::TableSetupColumn(label.c_str()); });
}

void WidgetBuilder::tableHeadersRow()
{
    addFunctionCall(ImGui::TableHeadersRow);
}

void WidgetBuilder::tableNextRow()
{
    addFunctionCall([] { ImGui::TableNextRow(); });
}

void WidgetBuilder::tableSetColumnIndex(const int32_t columnN)
{
    addInvocation<int32_t>(ImGui::TableSetColumnIndex, columnN);
}

sp<Boolean> WidgetBuilder::beginMainMenuBar()
{
    sp<Boolean::Impl> returnValue = sp<Boolean::Impl>::make(true);
    addWidgetGroupAndPush(sp<WidgetGroup>::make<MainMenuBar>(returnValue));
    return returnValue;
}

void WidgetBuilder::endMainMenuBar()
{
    pop();
}

sp<Boolean> WidgetBuilder::beginMenu(String label, sp<Boolean> enabled)
{
    sp<Boolean::Impl> returnValue = sp<Boolean::Impl>::make(true);
    addWidgetGroupAndPush(sp<WidgetGroup>::make<Menu>(std::move(label), std::move(enabled), returnValue));
    return returnValue;
}

void WidgetBuilder::endMenu()
{
    pop();
}

sp<Observer> WidgetBuilder::menuItem(String label, String shortcut, bool pSelected, sp<Boolean> enabled)
{
    sp<Observer> observer = sp<Observer>::make();
    addWidget(sp<Widget>::make<MenuItem>(std::move(label), std::move(shortcut), sp<Boolean>::make<Boolean::Const>(pSelected), std::move(enabled), observer, false));
    return observer;
}

sp<Observer> WidgetBuilder::menuItem(String label, String shortcut, sp<Boolean> pSelected, sp<Boolean> enabled)
{
    sp<Observer> observer = sp<Observer>::make();
    addWidget(sp<Widget>::make<MenuItem>(std::move(label), std::move(shortcut), std::move(pSelected), std::move(enabled), observer, true));
    return observer;
}

void WidgetBuilder::pushID(const String& id)
{
    addFunctionCall([id] { ImGui::PushID(id.c_str()); } );
}

void WidgetBuilder::pushID(const int64_t id)
{
    int32_t idHash = static_cast<int32_t>(id) ^ static_cast<int32_t>(id >> 32);
    addFunctionCall([idHash] { ImGui::PushID(idHash); } );
}

void WidgetBuilder::popID()
{
    addFunctionCall(ImGui::PopID);
}

void WidgetBuilder::image(sp<Texture> texture, sp<Vec2> size, const V2& uv0, const V2& uv1, sp<Vec4> color, sp<Vec4> borderColor)
{
    addWidget(sp<Widget>::make<Image>(std::move(texture), std::move(size), uv0, uv1, std::move(color), std::move(borderColor), _renderer_context));
}

sp<Widget> WidgetBuilder::sameLine(float offsetFromStartX, float spacing)
{
    sp<Widget> widget = WidgetType::create(sp<Widget>::make<FunctionCall>([offsetFromStartX, spacing] { ImGui::SameLine(offsetFromStartX, spacing); }));
    addWidget(widget);
    return widget;
}

sp<Widget> WidgetBuilder::newLine()
{
    sp<Widget> widget = WidgetType::create(sp<Widget>::make<FunctionCall>(ImGui::NewLine));
    addWidget(widget);
    return widget;
}

void WidgetBuilder::separator()
{
    addFunctionCall(ImGui::Separator);
}

void WidgetBuilder::separatorText(String label)
{
    addInvocation<String>(ImGui::SeparatorText, std::move(label));
}

void WidgetBuilder::spacing()
{
    addFunctionCall(ImGui::Spacing);
}

sp<Boolean> WidgetBuilder::beginTabBar(String strId, const Imgui::ImGuiTabBarFlags flags)
{
    sp<BooleanWrapper> isOpen = sp<BooleanWrapper>::make(false);
    addWidgetGroupAndPush(sp<WidgetGroup>::make<TabBar>(std::move(strId), flags, isOpen));
    return isOpen;
}

void WidgetBuilder::endTabBar()
{
    pop();
}

sp<Boolean> WidgetBuilder::beginTabItem(String label, const sp<Boolean>& pOpen, const Imgui::ImGuiTabItemFlags flags)
{
    DCHECK_WARN(!pOpen || pOpen.isInstance<BooleanWrapper>(), "p_open should be a BooleanWrapper instance");
    sp<BooleanWrapper> activated = sp<BooleanWrapper>::make(false);
    addWidgetGroupAndPush(sp<WidgetGroup>::make<TabItem>(std::move(label), flags, activated, pOpen.asInstance<BooleanWrapper>()));
    return activated;
}

void WidgetBuilder::endTabItem()
{
    pop();
}

sp<Widget> WidgetBuilder::makeAboutWidget(sp<Boolean> isOpen)
{
    return sp<Widget>::make<DemoWindow>(ImGui::ShowAboutWindow, std::move(isOpen));
}

sp<Widget> WidgetBuilder::makeDemoWidget(sp<Boolean> isOpen)
{
    return sp<Widget>::make<DemoWindow>(ImGui::ShowDemoWindow, std::move(isOpen));
}

void WidgetBuilder::addWidgetGroupAndPush(sp<WidgetGroup> widgetGroup)
{
    addWidget(widgetGroup);
    push(std::move(widgetGroup));
}

void WidgetBuilder::addFunctionCall(std::function<void()> func)
{
    addWidget(sp<Widget>::make<FunctionCall>(std::move(func)));
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
