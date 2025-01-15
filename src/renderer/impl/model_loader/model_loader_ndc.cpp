#include "renderer/impl/model_loader/model_loader_ndc.h"

#include "core/types/global.h"

#include "renderer/base/model.h"
#include "renderer/base/shader.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements.h"

namespace ark {

ModelLoaderNDC::ModelLoaderNDC()
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, nullptr)
{
}

sp<RenderCommandComposer> ModelLoaderNDC::makeRenderCommandComposer(const Shader& shader)
{
    _unit_model = Global<Constants>()->MODEL_NDC;
    return sp<RenderCommandComposer>::make<RCCDrawElements>(_unit_model);
}

sp<Model> ModelLoaderNDC::loadModel(int32_t type)
{
    return _unit_model;
}

sp<ModelLoader> ModelLoaderNDC::BUILDER::build(const Scope& args)
{
    return sp<ModelLoader>::make<ModelLoaderNDC>();
}

}
