#include "renderer/impl/layer/image_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/gl_shader.h"
#include "renderer/impl/gl_model_loader/gl_model_loader_quad.h"
#include "renderer/impl/layer/gl_model_layer.h"

namespace ark {

ImageLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext)) {
}

sp<Layer> ImageLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<GLModelLayer>::make(sp<GLModelLoaderQuad>::make(), _shader->build(args), atlas, _resource_loader_context);
}

}
