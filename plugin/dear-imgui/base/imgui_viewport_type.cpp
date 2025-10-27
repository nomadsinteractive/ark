#include "dear-imgui/base/imgui_viewport_type.h"

#include <imgui.h>

#include "graphics/impl/vec/vec2_impl.h"

namespace ark::plugin::dear_imgui {

namespace {

class IntegerImGuiViewportId final : public Integer {
public:
    IntegerImGuiViewportId(const ImGuiViewport& viewport)
        : _viewport(viewport)
    {
    }

    bool update(uint64_t timestamp) override
    {
        return true;
    }

    int32_t val() override
    {
        return _viewport.ID;
    }

private:
    const ImGuiViewport& _viewport;
};

class Vec2ImGuiViewportProp final : public Vec2 {
public:
    Vec2ImGuiViewportProp(const ImVec2& property)
        : _property(property)
    {
    }

    bool update(uint64_t timestamp) override
    {
        return true;
    }

    V2 val() override
    {
        return {_property.x, _property.y};
    }

private:
    const ImVec2& _property;
};

}

ImGuiViewportType::ImGuiViewportType(ImGuiViewport* viewport)
    : _imgui_viewport(viewport)
{
}

sp<Integer> ImGuiViewportType::id() const
{
    return sp<Integer>::make<IntegerImGuiViewportId>(*_imgui_viewport);
}

sp<Vec2> ImGuiViewportType::pos() const
{
    return sp<Vec2>::make<Vec2ImGuiViewportProp>(_imgui_viewport->Pos);
}

sp<Vec2> ImGuiViewportType::size() const
{
    return sp<Vec2>::make<Vec2ImGuiViewportProp>(_imgui_viewport->Size);
}

sp<Vec2> ImGuiViewportType::workPos() const
{
    return sp<Vec2>::make<Vec2ImGuiViewportProp>(_imgui_viewport->WorkPos);
}

sp<Vec2> ImGuiViewportType::workSize() const
{
    return sp<Vec2>::make<Vec2ImGuiViewportProp>(_imgui_viewport->WorkSize);
}

}
