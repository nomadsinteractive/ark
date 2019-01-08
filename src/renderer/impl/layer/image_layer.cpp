#include "renderer/impl/layer/image_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer.h"

#include "renderer/base/shader.h"
#include "renderer/impl/render_model/render_model_quad.h"

namespace ark {

ImageLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> ImageLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<Layer>::make(sp<GLModelQuad>::make(atlas), _shader->build(args), _resource_loader_context);
}

}
