#include "dear-imgui/base/renderer_context.h"

#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader.h"
#include "renderer/base/texture.h"

#include "dear-imgui/base/draw_command_pool.h"


namespace ark {
namespace plugin {
namespace dear_imgui {

RendererContext::RendererContext(const sp<Shader>& shader, const sp<RenderController>& renderController)
    : _shader(shader), _render_controller(renderController)
{
}

void RendererContext::addDefaultTexture(const sp<Texture>& texture)
{
    _draw_commands[nullptr] = sp<DrawCommandPool>::make(_shader, _render_controller, texture);
}

const sp<DrawCommandPool>& RendererContext::obtainDrawCommandPool(void* texture) const
{
    const auto iter = _draw_commands.find(texture);
    DCHECK(iter != _draw_commands.end(), "No Texture(%p) drawing requested", texture);
    return iter->second;
}

void RendererContext::addTextureRefCount(Texture* texture)
{
    DCHECK(!texture, "May not add default texture refcount");
    sp<DrawCommandPool>& pool = _draw_commands[texture];
    if(!pool)
        pool = sp<DrawCommandPool>::make(_shader, _render_controller, sp<Texture>::make(*texture));
    ++ pool->_refcount;
}

void RendererContext::relTextureRefCount(Texture* texture)
{
    DCHECK(!texture, "May not release default texture refcount");
    const auto iter = _draw_commands.find(texture);
    DASSERT(iter != _draw_commands.end());
    sp<DrawCommandPool>& pool = iter->second;
    if(-- pool->_refcount <= 0)
        _draw_commands.erase(iter);
}

}
}
}
