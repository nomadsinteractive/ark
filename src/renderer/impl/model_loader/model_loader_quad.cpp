#include "renderer/impl/model_loader/model_loader_quad.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/render_controller.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements_incremental.h"
#include "renderer/impl/vertices/vertices_quad.h"
#include "renderer/util/render_util.h"

namespace ark {

ModelLoaderQuad::ModelLoaderQuad(sp<Atlas> atlas)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES, atlas->texture()), _atlas(std::move(atlas)), _unit_model(Global<Constants>()->MODEL_UNIT_QUAD)
{
}

sp<RenderCommandComposer> ModelLoaderQuad::makeRenderCommandComposer(const Shader& /*shader*/)
{
    return Ark::instance().renderController()->makeDrawElementsIncremental(_unit_model);
}

sp<Model> ModelLoaderQuad::loadModel(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    return sp<Model>::make(_unit_model->indices(), sp<VerticesQuad>::make(texCoord), sp<Boundaries>::make(V3(0), V3(size, 0), V3(texCoord.pivot(), 0)));
}

ModelLoaderQuad::BUILDER::BUILDER(BeanFactory& factory, const String& atlas)
    : _atlas(factory.ensureBuilder<Atlas>(atlas))
{
}

sp<ModelLoader> ModelLoaderQuad::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderQuad>::make(_atlas->build(args));
}

}
