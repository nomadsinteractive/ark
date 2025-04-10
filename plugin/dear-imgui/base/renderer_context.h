#pragma once

#include "core/types/shared_ptr.h"

#include "renderer/forwarding.h"

#include "dear-imgui/forwarding.h"

namespace ark::plugin::dear_imgui {

class RendererContext {
public:
    RendererContext(const sp<Shader>& shader);

    void addDefaultTexture(sp<Texture> texture);
    const sp<DrawCommandPool>& obtainDrawCommandPool(void* texture) const;

    void addTextureRef(const sp<Texture>& texture);
    void releaseTextureRef(const sp<Texture>& texture);

private:
    sp<Shader> _shader;

    Map<void*, sp<DrawCommandPool>> _draw_commands;
};

}
