#include "renderer/impl/layer/point_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer.h"

#include "renderer/base/shader.h"
#include "renderer/impl/gl_model/gl_model_point.h"

namespace ark {

PointLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/point.vert", "shaders/point.frag")) {
}

sp<Layer> PointLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<Layer>::make(sp<GLModelPoint>::make(atlas), _shader->build(args), _resource_loader_context);
}

}
