#include "graphics/base/render_object_with_layer.h"

#include "core/base/bean_factory.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"
#include "graphics/base/boundaries.h"
#include "graphics/base/render_object.h"
#include "graphics/base/render_layer.h"
#include "graphics/base/size.h"

#include "renderer/base/model.h"
#include "renderer/inf/model_loader.h"

namespace ark {

RenderObjectWithLayer::RenderObjectWithLayer(sp<LayerContext> layerContext, sp<RenderObject> renderObject)
    : _layer_context(std::move(layerContext)), _render_object(std::move(renderObject))
{
    DASSERT(_layer_context);
    if(!_render_object->_size)
        if(const sp<Boundaries>& boundaries = _layer_context->modelLoader()->loadModel(_render_object->type()->val())->content())
            _render_object->setSize(boundaries->size());
}

RenderObjectWithLayer::~RenderObjectWithLayer()
{
}

const sp<LayerContext>& RenderObjectWithLayer::layerContext() const
{
    return _layer_context;
}

const sp<RenderObject>& RenderObjectWithLayer::renderObject() const
{
    return _render_object;
}

RenderObjectWithLayer::BUILDER::BUILDER(BeanFactory& factory, const document& manifest)
    : _render_object(factory.ensureConcreteClassBuilder<RenderObject>(manifest, constants::RENDER_OBJECT)), _layer_context(sp<LayerContext::BUILDER>::make(factory, manifest, Layer::TYPE_UNSPECIFIED))
{
}

sp<RenderObjectWithLayer> RenderObjectWithLayer::BUILDER::build(const Scope& args)
{
    return sp<RenderObjectWithLayer>::make(_layer_context->build(args), _render_object->build(args));
}

}
