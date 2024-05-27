#include "with_layer.h"

#include "graphics/base/layer.h"
#include "graphics/base/layer_context.h"

namespace ark {

WithLayer::WithLayer(sp<Layer> layer)
    : _layer(std::move(layer))
{
}

void WithLayer::onWire(const Traits& components)
{
    if(const sp<RenderObject> renderObject = components.get<RenderObject>())
    {
        _layer->addRenderObject(renderObject);
    }
    else if(sp<Renderable> renderable = components.get<Renderable>())
    {
        const sp<Disposed> discarded = components.get<Disposed>();
        sp<Updatable> updatable = components.get<Updatable>();
        _layer->context()->add(std::move(renderable), std::move(updatable), discarded);
    }
}

}
