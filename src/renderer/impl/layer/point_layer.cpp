#include "renderer/impl/layer/point_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/render_layer.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/impl/render_model/render_model_point.h"

namespace ark {

PointLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _manifest(manifest), _resource_loader_context(resourceLoaderContext), _atlas(factory.ensureBuilder<Atlas>(manifest)),
      _shader(Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/point.vert", "shaders/point.frag")) {
}

sp<RenderLayer> PointLayer::BUILDER::build(const sp<Scope>& args)
{
    const sp<Atlas> atlas = _atlas->build(args);
    return sp<RenderLayer>::make(sp<GLModelPoint>::make(_resource_loader_context->renderController(), atlas), _shader->build(args), _resource_loader_context);
}

}
