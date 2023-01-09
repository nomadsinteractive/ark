#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_BY_RENDER_OBJECT_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_BY_RENDER_OBJECT_H_

#include "core/inf/builder.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"
#include "core/types/safe_ptr.h"

#include "graphics/forwarding.h"
#include "graphics/inf/block.h"
#include "graphics/inf/renderable.h"
#include "graphics/inf/renderer.h"
#include "graphics/impl/renderable/renderable_passive.h"

namespace ark {

//[[core::class]]
class RenderObjectWithLayer : public Renderer, public Block {
public:
    RenderObjectWithLayer(sp<LayerContext> layerContext, sp<RenderObject> renderObject);
    ~RenderObjectWithLayer() override;
[[deprecated]]
    virtual void render(RenderRequest& renderRequest, const V3& position) override;

    virtual const sp<Size>& size() override;

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


//  [[plugin::builder("render-object")]]
    class BUILDER_RENDERER : public Builder<Renderer> {
    public:
        BUILDER_RENDERER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const Scope& args) override;

    private:
        BUILDER _impl;
    };

private:
    void measure(Size& size);

private:
    sp<LayerContext> _layer_context;
    sp<RenderObject> _render_object;
    sp<RenderablePassive> _renderable;

};

}

#endif
