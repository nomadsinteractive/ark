#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_CONTEXT_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_CONTEXT_H_

#include <map>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class RendererContext {
public:
    RendererContext(const sp<Shader>& shader, const sp<RenderController>& renderController);

    void addDefaultTexture(sp<Texture> texture);
    const sp<DrawCommandPool>& obtainDrawCommandPool(void* texture) const;

    void addTextureRefCount(Texture* texture);
    void relTextureRefCount(Texture* texture);

private:
    sp<Shader> _shader;
    sp<RenderController> _render_controller;

    std::map<void*, sp<DrawCommandPool>> _draw_commands;
};

}
}
}

#endif
