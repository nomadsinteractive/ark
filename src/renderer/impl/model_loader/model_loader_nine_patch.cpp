#include "renderer/impl/model_loader/model_loader_nine_patch.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/render_command_composer/rcc_draw_quads.h"
#include "renderer/util/render_util.h"

namespace ark {

ModelLoaderNinePatch::ModelLoaderNinePatch(sp<Atlas> atlas)
    : ModelLoader(ModelLoader::RENDER_MODE_TRIANGLES), _atlas(std::move(atlas)), _nine_patch_attachment(_atlas->attachments().ensure<AtlasImporterNinePatch::Attachment>()),
      _unit_model(RenderUtil::makeUnitNinePatchModel())
{
}

sp<RenderCommandComposer> ModelLoaderNinePatch::makeRenderCommandComposer()
{
    return sp<RCCDrawQuads>::make(_unit_model);
}

void ModelLoaderNinePatch::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

void ModelLoaderNinePatch::postSnapshot(RenderController& /*renderController*/, RenderLayer::Snapshot& /*snapshot*/)
{
}

sp<Model> ModelLoaderNinePatch::loadModel(int32_t type)
{
    return sp<Model>::make(_unit_model.indices(), _nine_patch_attachment->ensureVertices(type));
}

ModelLoaderNinePatch::BUILDER::BUILDER(BeanFactory& factory, const String& atlas)
    : _atlas(factory.ensureBuilder<Atlas>(atlas))
{
}

sp<ModelLoader> ModelLoaderNinePatch::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderNinePatch>::make(_atlas->build(args));
}

ModelLoaderNinePatch::BUILDER_IMPL::BUILDER_IMPL(BeanFactory& factory, const document& manifest)
    : _builder(factory, Documents::ensureAttribute(manifest, Constants::Attributes::ATLAS))
{
}

sp<ModelLoader> ModelLoaderNinePatch::BUILDER_IMPL::build(const Scope& args)
{
    return _builder.build(args);
}

}
