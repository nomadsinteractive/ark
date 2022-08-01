#ifndef ARK_RENDERER_INF_RENDER_COMMAND_COMPOSER_H_
#define ARK_RENDERER_INF_RENDER_COMMAND_COMPOSER_H_

#include "core/base/api.h"

#include "renderer/forwarding.h"
#include "renderer/inf/model_loader.h"

namespace ark {

class ARK_API RenderCommandComposer {
public:
    virtual ~RenderCommandComposer() = default;

    virtual sp<ShaderBindings> makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode) = 0;
    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) = 0;

    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot) = 0;

};

}

#endif
