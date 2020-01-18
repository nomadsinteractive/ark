#include "renderer/impl/layer/point_layer.h"

#include "renderer/base/shader.h"
#include "renderer/impl/model_loader/model_loader_point.h"
#include "renderer/impl/render_command_composer/rcc_uniform_models.h"
#include "renderer/util/element_util.h"

namespace ark {

PointLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext, sp<ModelLoaderPoint::BUILDER>::make(factory, manifest), sp<Builder<RenderCommandComposer>::Prebuilt>::make(sp<RCCUniformModels>::make(ElementUtil::makeUnitPointModel())),
            Shader::fromDocument(factory, manifest, resourceLoaderContext, "shaders/point.vert", "shaders/point.frag"))
{
}

sp<RenderLayer> PointLayer::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
