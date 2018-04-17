#include "renderer/impl/layer/nine_patch_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/gl_shader.h"
#include "renderer/impl/gl_model_loader/gl_model_loader_nine_patch.h"
#include "renderer/impl/layer/gl_model_layer.h"

namespace ark {

NinePatchLayer::BUILDER::BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(parent.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(parent, manifest, resourceLoaderContext))
{
}

sp<Layer> NinePatchLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    const sp<GLShader> shader = _shader->build(args);
    return sp<GLModelLayer>::make(sp<GLModelLoaderNinePatch>::make(_manifest, atlas), shader, atlas, _resource_loader_context);
}

}
