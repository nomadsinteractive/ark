#ifndef ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_UNIFORM_MODELS_H_
#define ARK_RENDERER_IMPL_RENDER_COMMAND_COMPOSER_RCC_UNIFORM_MODELS_H_


#include "renderer/forwarding.h"
#include "renderer/base/model.h"
#include "renderer/inf/render_command_composer.h"

namespace ark {

class RCCUniformModels : public RenderCommandComposer {
public:
    RCCUniformModels(Model model);

    virtual sp<ShaderBindings> makeShaderBindings(Shader& shader, RenderController& renderController, RenderModel::Mode renderMode) override;

    virtual void postSnapshot(RenderController& renderController, RenderLayer::Snapshot& snapshot) override;
    virtual sp<RenderCommand> compose(const RenderRequest& renderRequest, RenderLayer::Snapshot& snapshot) override;

private:
    Model _model;
    sp<NamedBuffer> _shared_buffer;
};

}

#endif
