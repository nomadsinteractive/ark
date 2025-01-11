#include "renderer/impl/model_loader/model_loader_unit_quad.h"

#include "core/types/global.h"

#include "renderer/base/model.h"
#include "renderer/base/shader.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements.h"

namespace ark {

ModelLoaderUnitQuad::ModelLoaderUnitQuad()
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, nullptr)
{
}

sp<RenderCommandComposer> ModelLoaderUnitQuad::makeRenderCommandComposer(const Shader& shader)
{
    _unit_model = shader.input()->camera().isLHS() ? Global<Constants>()->MODEL_UNIT_QUAD_LHS : Global<Constants>()->MODEL_UNIT_QUAD_RHS;
    return sp<RCCDrawElements>::make(_unit_model);
}

sp<Model> ModelLoaderUnitQuad::loadModel(int32_t type)
{
    return _unit_model;
}

sp<ModelLoader> ModelLoaderUnitQuad::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderUnitQuad>::make();
}

}
