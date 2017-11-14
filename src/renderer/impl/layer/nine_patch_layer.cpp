#include "renderer/impl/layer/nine_patch_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/atlas.h"
#include "renderer/base/gl_texture.h"
#include "renderer/base/gl_shader.h"
#include "renderer/impl/gl_model/gl_model_nine_patch.h"
#include "renderer/base/resource_loader_context.h"

namespace ark {

NinePatchLayer::NinePatchLayer(const sp<Atlas>& atlas, const sp<GLShader>& shader, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _nine_patch_elements(shader, atlas->texture(), sp<GLModelNinePatch>::make(shader, manifest, atlas), resourceLoaderContext)
{
}

sp<RenderCommand> NinePatchLayer::render(const LayerContext& renderContext, float x, float y)
{
    return _nine_patch_elements.render(renderContext, x, y);
}

NinePatchLayer::BUILDER::BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(parent.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(parent, manifest, resourceLoaderContext))
{
}

sp<Layer> NinePatchLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    const sp<GLShader> shader = _shader->build(args);
    return sp<NinePatchLayer>::make(atlas, shader, _manifest, _resource_loader_context);
}

}
