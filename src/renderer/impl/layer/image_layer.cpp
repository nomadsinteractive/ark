#include "renderer/impl/layer/image_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/model_loader/model_loader_quad.h"

namespace ark {

ImageLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext, sp<ModelLoaderQuad::BUILDER>::make(factory, manifest)) {
}

sp<RenderLayer> ImageLayer::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
