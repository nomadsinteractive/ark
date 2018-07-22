#include "renderer/impl/layer/point_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/gl_shader.h"
#include "renderer/impl/gl_model_loader/gl_model_loader_point.h"
#include "renderer/impl/layer/gl_model_layer.h"

namespace ark {

PointLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(GLShader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/point.vert", "shaders/point.frag")) {
}

sp<Layer> PointLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<GLModelLayer>::make(sp<GLModelLoaderPoint>::make(), _shader->build(args), atlas, _resource_loader_context);
}

}
