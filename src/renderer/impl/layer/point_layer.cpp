#include "renderer/impl/layer/point_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/resource_loader_context.h"
#include "renderer/base/shader.h"
#include "renderer/impl/render_model/render_model_point.h"

namespace ark {

PointLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext, sp<RenderModelPoint::BUILDER>::make(factory, manifest, resourceLoaderContext), nullptr,
            Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/point.vert", "shaders/point.frag"))
{
}

sp<RenderLayer> PointLayer::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
