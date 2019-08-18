#ifndef ARK_GRAPHICS_IMPL_RENDERER_RENDERER_BY_RENDER_OBJECT_H_
#define ARK_GRAPHICS_IMPL_RENDERER_RENDERER_BY_RENDER_OBJECT_H_

#include "core/inf/builder.h"
#include "core/types/class.h"
#include "core/types/shared_ptr.h"

#include "graphics/inf/block.h"
#include "graphics/inf/renderer.h"
#include "graphics/forwarding.h"

namespace ark {

//[[core::class]]
class RendererByRenderObject : public Renderer, public Block {
public:
    RendererByRenderObject(const sp<RenderObject>& renderObject, const sp<LayerContext>& layerContext);

    virtual void render(RenderRequest& renderRequest, float x, float y) override;

    virtual const SafePtr<Size>& size() override;

//  [[plugin::builder("render-object")]]
    class BUILDER : public Builder<Renderer> {
    public:
        BUILDER(BeanFactory& factory, const document& manifest);

        virtual sp<Renderer> build(const sp<Scope>& args) override;

    private:
        sp<Builder<RenderObject>> _render_object;
        sp<Builder<LayerContext>> _layer_context;
    };

private:
    void measure(Size& size);

private:
    sp<RenderObject> _render_object;
    sp<LayerContext> _layer_context;

};

}

#endif
