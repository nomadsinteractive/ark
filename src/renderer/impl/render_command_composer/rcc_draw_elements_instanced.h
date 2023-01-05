#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_DRAW_ELEMENTS_INSTANCED_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_DRAW_ELEMENTS_INSTANCED_H_

#include "renderer/forwarding.h"
#include "renderer/base/buffer.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCDrawElementsInstanced : public RenderCommandComposer {
public:
    RCCDrawElementsInstanced(Model model);

    virtual sp<ShaderBindings> makeShaderBindings(Shader& shader, RenderController& renderController, ModelLoader::RenderMode renderMode) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayerSnapshot& snapshot) override;
    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayerSnapshot& snapshot) override;

private:
    Model _model;
    Buffer _indices;
};

}

#endif
