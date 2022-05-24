#include "renderer/impl/model_loader/model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/render_command_composer/rcc_draw_quads.h"
#include "renderer/impl/vertices/vertices_quad.h"
#include "renderer/util/render_util.h"

namespace ark {

ModelLoaderQuad::ModelLoaderQuad(sp<Atlas> atlas)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLES), _atlas(std::move(atlas)), _unit_model(RenderUtil::makeUnitQuadModel())
{
}

sp<RenderCommandComposer> ModelLoaderQuad::makeRenderCommandComposer()
{
    return sp<RCCDrawQuads>::make(_unit_model);
}

void ModelLoaderQuad::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

sp<Model> ModelLoaderQuad::loadModel(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    return sp<Model>::make(_unit_model.indices(), sp<VerticesQuad>::make(texCoord), Metrics{V3(size, 0), V3(size, 0), V3(0)});
}

ModelLoaderQuad::MAKER::MAKER(BeanFactory& factory, const String& atlas)
    : _atlas(factory.ensureBuilder<Atlas>(atlas))
{
}

sp<ModelLoader> ModelLoaderQuad::MAKER::build(const Scope& args)
{
    return sp<ModelLoaderQuad>::make(_atlas->build(args));
}

}
