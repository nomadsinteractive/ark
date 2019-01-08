#include "renderer/impl/layer/nine_patch_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer.h"

#include "renderer/base/shader.h"
#include "renderer/impl/render_model/render_model_nine_patch.h"

namespace ark {

NinePatchLayer::BUILDER::BUILDER(BeanFactory& parent, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(parent.ensureBuilder<Atlas>(manifest)),
      _shader(Shader::fromDocument(parent, manifest, resourceLoaderContext))
{
}

sp<Layer> NinePatchLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    const sp<Shader> shader = _shader->build(args);
    return sp<Layer>::make(sp<GLModelNinePatch>::make(_manifest, atlas), shader, _resource_loader_context);
}

}
