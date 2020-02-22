#include "dear-imgui/base/renderer_context.h"


namespace ark {
namespace plugin {
namespace dear_imgui {

const sp<LFStack<sp<RendererImgui::DrawCommand>>>& RendererContext::obtainDrawCommandPool(void* texture) const
{
    if(texture == nullptr)
        return _default_pool._draw_commands;

    const auto iter = _draw_commands.find(texture);
    DCHECK(iter != _draw_commands.end(), "No Texture(%p) drawing requested");
    return iter->second._draw_commands;
}

void RendererContext::addTextureRefCount(Texture* texture)
{
    DrawCommandPool& pool = _draw_commands[texture];
    ++ pool._refcount;
}

void RendererContext::relTextureRefCount(Texture* texture)
{
    const auto iter = _draw_commands.find(texture);
    DASSERT(iter != _draw_commands.end());
    DrawCommandPool& pool = iter->second;
    if(-- pool._refcount <= 0)
        _draw_commands.erase(iter);
}

RendererContext::DrawCommandPool::DrawCommandPool()
    : _refcount(0), _draw_commands(sp<LFStack<sp<RendererImgui::DrawCommand>>>::make())
{
}

}
}
}
