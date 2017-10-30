#ifndef ARK_GRAPHICS_INF_LAYER_H_
#define ARK_GRAPHICS_INF_LAYER_H_

#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderer.h"

namespace ark {

//[[script::bindings::auto]]
class ARK_API Layer : public Renderer, Implements<Layer, Renderer> {
public:
    Layer();
    virtual ~Layer() = default;

    const sp<LayerContext>& renderContext() const;

    virtual void render(RenderCommandPipeline& pipeline, float x, float y) override final;

    virtual void render(const LayerContext& renderContext, RenderCommandPipeline& pipeline, float x, float y) = 0;

private:
    sp<LayerContext> _render_context;

};

}

#endif
