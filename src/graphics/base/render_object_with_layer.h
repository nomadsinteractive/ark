#pragma once

#include "core/inf/builder.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderer.h"

namespace ark {

//[[core::class]]
class RenderObjectWithLayer {
public:
    RenderObjectWithLayer(sp<LayerContext> layerContext, sp<RenderObject> renderObject);
    ~RenderObjectWithLayer();

    const sp<Size>& size();

    const sp<LayerContext>& layerContext() const;
    const sp<RenderObject>& renderObject() const;

//  [[plugin::builder]]
    class BUILDER : public Builder<RenderObjectWithLayer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<RenderObjectWithLayer> build(const Scope& args) override;

    private:
        sp<Builder<RenderObject>> _render_object;
        sp<Builder<LayerContext>> _layer_context;
    };

private:
    sp<LayerContext> _layer_context;
    sp<RenderObject> _render_object;

};

}
