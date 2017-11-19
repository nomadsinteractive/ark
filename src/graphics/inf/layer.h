#ifndef ARK_GRAPHICS_INF_LAYER_H_
#define ARK_GRAPHICS_INF_LAYER_H_

#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/base/layer_context.h"
#include "graphics/inf/renderer.h"

namespace ark {

//[[script::bindings::auto]]
//[[core::class]]
class ARK_API Layer {
public:
    Layer();
    virtual ~Layer() = default;

    const sp<LayerContext>& layerContext() const;

    virtual sp<RenderCommand> render(const LayerContext::Snapshot& layerContext, float x, float y) = 0;

    class Renderer : public ark::Renderer {
    public:
        Renderer(const sp<Layer>& layer);

        virtual sp<RenderCommand> render(RenderRequest& renderRequest, float x, float y) override;

    private:
        sp<Layer> _layer;
    };

private:
    sp<LayerContext> _layer_context;

};

}

#endif
