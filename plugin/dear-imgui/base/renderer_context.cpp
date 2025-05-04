#include "dear-imgui/base/renderer_context.h"

#include "core/ark.h"

#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "dear-imgui/base/draw_command_pool.h"

namespace ark::plugin::dear_imgui {

RendererContext::RendererContext(const sp<Shader>& shader)
    : _shader(shader)
{
}

void RendererContext::addDefaultTexture(sp<Texture> texture)
{
    _draw_commands[nullptr] = sp<DrawCommandPool>::make(_shader, Ark::instance().renderController(), std::move(texture));
}

const sp<DrawCommandPool>& RendererContext::obtainDrawCommandPool(void* texture) const
{
    const auto iter = _draw_commands.find(texture);
    DCHECK(iter != _draw_commands.end(), "No Texture(%p) drawing requested", texture);
    return iter->second;
}

void RendererContext::addTextureRef(const sp<Texture>& texture)
{
    ASSERT(texture);
    sp<DrawCommandPool>& pool = _draw_commands[texture.get()];
    if(!pool)
        pool = sp<DrawCommandPool>::make(_shader, Ark::instance().renderController(), texture);
    ++ pool->_refcount;
}

void RendererContext::releaseTextureRef(const sp<Texture>& texture)
{
    ASSERT(texture);
    const auto iter = _draw_commands.find(texture.get());
    DASSERT(iter != _draw_commands.end());
    sp<DrawCommandPool>& pool = iter->second;
    if(-- pool->_refcount <= 0)
        _draw_commands.erase(iter);
}

}
