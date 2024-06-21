#include "renderer/impl/model_loader/model_loader_nine_patch_quads.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_bindings.h"
#include "renderer/base/shader_bindings.h"
#include "renderer/impl/render_command_composer/rcc_draw_quads.h"
#include "renderer/util/render_util.h"

namespace ark {

ModelLoaderNinePatchQuads::ModelLoaderNinePatchQuads(sp<Atlas> atlas)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLES), _atlas(std::move(atlas)), _nine_patch_attachment(_atlas->attachments().ensure<Atlas::AttachmentNinePatch>()),
      _unit_model(RenderUtil::makeUnitNinePatchQuadsModel())
{
}

sp<RenderCommandComposer> ModelLoaderNinePatchQuads::makeRenderCommandComposer()
{
    return sp<RCCDrawQuads>::make(_unit_model);
}

void ModelLoaderNinePatchQuads::initialize(ShaderBindings& shaderBindings)
{
    shaderBindings.pipelineBindings()->bindSampler(_atlas->texture());
}

sp<Model> ModelLoaderNinePatchQuads::loadModel(int32_t type)
{
    return sp<Model>::make(_unit_model.indices(), _nine_patch_attachment->ensureVerticesQuads(type), _unit_model.content(), _unit_model.occupy());
}

ModelLoaderNinePatchQuads::BUILDER::BUILDER(BeanFactory& factory, const String& atlas)
    : _atlas(factory.ensureBuilder<Atlas>(atlas))
{
}

sp<ModelLoader> ModelLoaderNinePatchQuads::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderNinePatchQuads>::make(_atlas->build(args));
}

ModelLoaderNinePatchQuads::BUILDER_IMPL::BUILDER_IMPL(BeanFactory& factory, const document& manifest)
    : _builder(factory, Documents::ensureAttribute(manifest, constants::ATLAS))
{
}

sp<ModelLoader> ModelLoaderNinePatchQuads::BUILDER_IMPL::build(const Scope& args)
{
    return _builder.build(args);
}

}
