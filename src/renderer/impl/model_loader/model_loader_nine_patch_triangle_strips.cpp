#include "renderer/impl/model_loader/model_loader_nine_patch_triangle_strips.h"

#include "core/types/global.h"

#include "renderer/base/atlas.h"
#include "renderer/base/model.h"
#include "renderer/base/pipeline_descriptor.h"
#include "renderer/base/shader.h"
#include "renderer/impl/render_command_composer/rcc_draw_elements.h"
#include "renderer/util/render_util.h"

namespace ark {

ModelLoaderNinePatchTriangleStrips::ModelLoaderNinePatchTriangleStrips(sp<Atlas> atlas)
    : ModelLoader(Enum::RENDER_MODE_TRIANGLE_STRIP, atlas->texture()), _atlas(std::move(atlas)), _nine_patch_attachment(_atlas->attachments().ensure<Atlas::AttachmentNinePatch>()),
      _unit_model(Global<Constants>()->MODEL_UNIT_NINE_PATCH_TRIANGLE_STRIPS)
{
}

sp<RenderCommandComposer> ModelLoaderNinePatchTriangleStrips::makeRenderCommandComposer(const Shader& shader)
{
    _is_lhs = shader.input()->camera().isLHS();
    return sp<RCCDrawElements>::make(_unit_model);
}

sp<Model> ModelLoaderNinePatchTriangleStrips::loadModel(int32_t type)
{
    return sp<Model>::make(_unit_model->indices(), _nine_patch_attachment->ensureVerticesTriangleStrips(type, _is_lhs), _unit_model->content(), _unit_model->occupy());
}

ModelLoaderNinePatchTriangleStrips::BUILDER::BUILDER(BeanFactory& factory, const String& atlas)
    : _atlas(factory.ensureBuilder<Atlas>(atlas))
{
}

sp<ModelLoader> ModelLoaderNinePatchTriangleStrips::BUILDER::build(const Scope& args)
{
    return sp<ModelLoaderNinePatchTriangleStrips>::make(_atlas->build(args));
}

}
