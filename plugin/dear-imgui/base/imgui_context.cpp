#include "dear-imgui/base/imgui_context.h"


namespace ark {
namespace plugin {
namespace dear_imgui {

ImguiContext::ImguiContext()
    : _context(ImGui::CreateContext()) {
}

ImguiContext::~ImguiContext()
{
    ImGui::DestroyContext(_context);
}

const sp<LFStack<sp<RendererImgui::DrawCommand>>>& ImguiContext::obtainDrawCommandPool(void* texture) const
{
    if(texture == nullptr)
        return _default_pool._draw_commands;

    const auto iter = _draw_commands.find(texture);
    DCHECK(iter != _draw_commands.end(), "No Texture(%p) drawing requested");
    return iter->second._draw_commands;
}

void ImguiContext::addTextureRefCount(Texture* texture)
{
    DrawCommandPool& pool = _draw_commands[texture];
    ++ pool._refcount;
}

void ImguiContext::relTextureRefCount(Texture* texture)
{
    const auto iter = _draw_commands.find(texture);
    DASSERT(iter != _draw_commands.end());
    DrawCommandPool& pool = iter->second;
    if(-- pool._refcount <= 0)
        _draw_commands.erase(iter);
}

ImguiContext::DrawCommandPool::DrawCommandPool()
    : _refcount(0), _draw_commands(sp<LFStack<sp<RendererImgui::DrawCommand>>>::make())
{
}

}
}
}
