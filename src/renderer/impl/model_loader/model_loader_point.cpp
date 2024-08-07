#include "renderer/impl/model_loader/model_loader_point.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements.h"
#include "renderer/impl/vertices/vertices_point.h"
#include "renderer/util/render_util.h"


namespace ark {

ModelLoaderPoint::ModelLoaderPoint(const sp<Atlas>& atlas)
    : ModelLoader(Enum::RENDER_MODE_POINTS), _atlas(atlas)
{
}

sp<RenderCommandComposer> ModelLoaderPoint::makeRenderCommandComposer()
{
    return sp<RCCDrawElements>::make(Global<Constants>()->MODEL_UNIT_POINT);
}

void ModelLoaderPoint::initialize(PipelineBindings& pipelineBindings)
{
    pipelineBindings.pipelineDescriptor()->bindSampler(_atlas->texture());
}

sp<Model> ModelLoaderPoint::loadModel(int32_t type)
{
    const Atlas::Item& texCoord = _atlas->at(type);
    const V2& size = texCoord.size();
    return sp<Model>::make(nullptr, sp<VerticesPoint>::make(texCoord), sp<Boundaries>::make(V3(0), V3(size, 0), V3(texCoord.pivot(), 0)));
}

ModelLoaderPoint::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _atlas(factory.ensureConcreteClassBuilder<Atlas>(manifest, constants::ATLAS))
{
}

sp<ModelLoader> ModelLoaderPoint::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderPoint>::make(_atlas->build(args));
}

}
