#include "renderer/impl/model_loader/model_loader_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/render_command_composer/rcc_uniform_models.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/util/element_util.h"


namespace ark {

ModelLoaderPoint::ModelLoaderPoint(const sp<Atlas>& atlas)
    : ModelLoader(ModelLoader::RENDER_MODE_POINTS), _atlas(atlas)
{
}

sp<RenderCommandComposer> ModelLoaderPoint::makeRenderCommandComposer()
{
    return sp<RCCUniformModels>::make(ElementUtil::makeUnitPointModel());
}

void ModelLoaderPoint::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderPoint::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

Model ModelLoaderPoint::load(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    return Model(nullptr, sp<VerticesPoint>::make(texCoord), {V3(size, 0), V3(size, 0), V3(0)});
}

ModelLoaderPoint::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, Constants::Attributes::ATLAS))
{
}

sp<ModelLoader> ModelLoaderPoint::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderPoint>::make(_atlas->build(args));
}

}
