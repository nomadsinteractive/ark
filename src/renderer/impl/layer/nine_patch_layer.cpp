#include "renderer/impl/layer/nine_patch_layer.h"

#include "core/base/bean_factory.h"

#include "renderer/base/shader.h"
#include "renderer/base/resource_loader_context.h"
#include "renderer/impl/model_loader/model_loader_nine_patch.h"
#include "renderer/impl/render_command_composer/rcc_uniform_models.h"
#include "renderer/util/element_util.h"


namespace ark {

NinePatchLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest, const sp<ResourceLoaderContext>& resourceLoaderContext)
    : _impl(factory, manifest, resourceLoaderContext, sp<ModelLoaderNinePatch::BUILDER>::make(factory, manifest), sp<Builder<RenderCommandComposer>::Prebuilt>::make(sp<RCCUniformModels>::make(ElementUtil::makeUnitNinePatchModel())))
{
}

sp<RenderLayer> NinePatchLayer::BUILDER::build(const Scope& args)
{
    return _impl.build(args);
}

}
