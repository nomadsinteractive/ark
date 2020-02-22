#ifndef ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_CONTEXT_H_
#define ARK_PLUGIN_DEAR_IMGUI_BASE_RENDERER_CONTEXT_H_

#include <map>

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"
#include "dear-imgui/api.h"

#include "dear-imgui/renderer/renderer_imgui.h"

namespace ark {
namespace plugin {
namespace dear_imgui {

class RendererContext {
public:
    const sp<LFStack<sp<RendererImgui::DrawCommand>>>& obtainDrawCommandPool(void* texture) const;

    void addTextureRefCount(Texture* texture);
    void relTextureRefCount(Texture* texture);

    void clear();

private:
    struct DrawCommandPool {
        DrawCommandPool();

        int32_t _refcount;
        sp<LFStack<sp<RendererImgui::DrawCommand>>> _draw_commands;
    };

private:
    std::map<void*, DrawCommandPool> _draw_commands;
    DrawCommandPool _default_pool;

};

}
}
}

#endif
